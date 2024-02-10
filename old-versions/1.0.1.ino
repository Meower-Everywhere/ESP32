#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>

Preferences preferences;

const char* meowerAPI = "https://api.meower.org/home?autoget=1&page=1";
const char* version = "1.0.1";
unsigned long lastTimestamp = 0;
bool firstRun = true;

void setup() {
  Serial.begin(115200);
  while (!Serial) continue; // Wait for Serial to be ready
  Serial.println("---------------- Meower for ESP32 ----------------");
  Serial.print("Created with ❤️ by JoshAtticus | Version ");
  Serial.println(version);
  Serial.println("--------------------------------------------------");

  initializeWiFi(); // Initialize WiFi connection
  fetchPosts(); // Fetch posts after connecting to WiFi
}

void loop() {
  // Fetch new posts every second, if connected
  if (WiFi.status() == WL_CONNECTED) {
    fetchPosts();
  } else {
    Serial.println("WiFi not connected. Attempting to reconnect...");
    initializeWiFi();
  }

  // Check for serial input to handle "wifi", "post: ", or "username: " commands
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "wifi") {
      // Clear existing credentials and restart WiFi setup
      preferences.begin("wifi", false);
      preferences.clear();
      preferences.end();
      initializeWiFi();
    } else if (input.startsWith("post: ")) {
      String content = input.substring(6); // Extract content after "post: "
      sendPost(content);
    } else if (input.startsWith("username: ")) {
      String newUsername = input.substring(10); // Extract username after "username: "
      setUsername(newUsername);
    }
  }

  delay(500);
}

void setUsername(const String& newUsername) {
  if (newUsername.length() > 0) {
    // Save the new username to preferences
    preferences.begin("meower", false);
    preferences.putString("username", newUsername);
    preferences.end(); // Data is saved, close the preferences

    // Print a confirmation message
    Serial.print("Username set to: ");
    Serial.println(newUsername);
  } else {
    Serial.println("Username cannot be blank.");
  }
}

void initializeWiFi() {
  preferences.begin("wifi", false);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end(); // Close the Preferences before connecting

  if (ssid == "" || password == "") {
    askForWiFiCredentials();
  } else {
    connectToWiFi(ssid, password);
  }

  preferences.begin("meower", false); // Use a separate namespace for the username
  String username = preferences.getString("username", "");
  preferences.end();

  if (username == "") {
    askForUsername();
  }
}


void askForUsername() {
  Serial.println("Enter Username (must not be blank!):");
  while (!Serial.available()) { delay(100); } // Wait for input
  String username = Serial.readStringUntil('\n');
  username.trim(); // Remove any newline or whitespace

  if (username.length() > 0) {
    // Save Username to preferences under "meower" namespace
    preferences.begin("meower", false);
    preferences.putString("username", username);
    preferences.end(); // Data is saved, close the preferences

    Serial.print("Username set to: ");
    Serial.println(username);
  } else {
    Serial.println("Username cannot be blank.");
  }
}

void askForWiFiCredentials() {
  // Clear any existing credentials
  preferences.clear();

  // Clear the serial buffer to ensure there's no leftover input
  while (Serial.available() > 0) {
    Serial.read();
  }

  Serial.println("Enter SSID:");
  while (!Serial.available()) { delay(100); } // Wait for input
  String ssid = Serial.readStringUntil('\n');
  ssid.trim(); // Remove any newline or whitespace

  Serial.println("Enter Password:");
  while (!Serial.available()) { delay(100); } // Wait for input
  String password = Serial.readStringUntil('\n');
  password.trim(); // Remove any newline or whitespace

  // Save SSID and Password to preferences
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.putBool("configured", true);

  // Explicitly end the preferences to commit them to NVS
  preferences.end();

  // Give some time for NVS to save the preferences
  delay(500);

  Serial.println("WiFi credentials saved.");

  // Attempt to connect with the new credentials
  connectToWiFi(ssid, password);
}

void connectToWiFi(const String& ssid, const String& password) {
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.print("Connecting to WiFi");
  unsigned long startAttemptTime = millis();

  // Set a timeout period for the connection attempt (e.g., 15 seconds)
  unsigned long connectionTimeout = 15 * 1000; // 15,000 milliseconds

  // Attempt to connect until the timeout is reached
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startAttemptTime < connectionTimeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" connected!");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("--------------------------------------------------");
  } else {
    Serial.println(" connection failed!");
    WiFi.disconnect(); // Disconnect from the WiFi network
    askForWiFiCredentials(); // Ask for credentials again
  }
}

void sendPost(const String& content) {
  preferences.begin("meower", true); // Open "meower" namespace to retrieve the username
  String username = preferences.getString("username", "");
  preferences.end(); // Close the preferences after retrieving the username

  if (username != "" && content != "") {
    // Prepare the JSON payload
    DynamicJsonDocument doc(1024);
    doc["post"] = content;
    doc["username"] = username;
    String requestBody;
    serializeJson(doc, requestBody);

    // Send the POST request
    HTTPClient http;
    http.begin("https://webhooks.meower.org/post/home");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      Serial.println("Post sent successfully!");
    } else {
      Serial.print("Error sending post: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Username or content missing.");
  }
}


void fetchPosts() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(meowerAPI);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      // Parse JSON
      DynamicJsonDocument doc(8192); // Adjust size for your needs
      deserializeJson(doc, payload);
      JsonArray posts = doc["autoget"].as<JsonArray>();

      // Assume the first post is the latest; adjust if a newer one is found
      JsonObject latestPost = posts[0];
      unsigned long latestTimestamp = latestPost["t"]["e"].as<unsigned long>();

      // Find the latest post in the current fetched posts
      for (JsonObject post : posts) {
        unsigned long postTimestamp = post["t"]["e"].as<unsigned long>();
        if (postTimestamp > latestTimestamp) {
          latestTimestamp = postTimestamp;
          latestPost = post; // Update the latest post
        }
      }

      // Check if the latest post is newer than the last one we've seen
      if (latestTimestamp > lastTimestamp) {
        String username = latestPost["u"].as<String>();
        String message = latestPost["p"].as<String>();

        // Check if the username is "Discord", "Revower", or "Webhooks"
        bool isBridgedUser = username == "Discord" || username == "Revower";
        bool isWebhookUser = username == "Webhooks";
        bool isESP32User = username == "ESP32Meower";
        if (isBridgedUser) {
          username = "(BRIDGED)"; // Replace with "(BRIDGED)"
        } else if (isWebhookUser) {
          username = "(WEBHOOK)"; // Replace with "(WEBHOOK)"
        }

        // Print the latest post
        Serial.print(username);
        // Only print the colon if it's not a special user
        if (!isBridgedUser && !isWebhookUser) {
          Serial.print(": ");
        } else {
          Serial.print(" "); // Print a space after the special tag
        }
        Serial.println(message);

        // Update lastTimestamp with the latest post's timestamp
        lastTimestamp = latestTimestamp;
      }

    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  } else {
    Serial.println("WiFi not connected. Reconnecting...");
    reconnectWiFi();
  }
}


void reconnectWiFi() {
  // Retrieve credentials from preferences
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid");
  String password = preferences.getString("password");
  preferences.end();

  // Reconnect to WiFi
  WiFi.begin(ssid.c_str(), password.c_str());
}
