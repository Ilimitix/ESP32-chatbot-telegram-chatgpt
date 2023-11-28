#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* OPENAI_CERTIFICATE_ROOT = //récupérer le certificat ssl d'openai et l'insérer ici
R"(-----BEGIN CERTIFICATE----- 
-----END CERTIFICATE-----)";

const char *ssid = "votressid"; //votre ssid
const char *password = "votremdp"; //votre mdp
#define BOTtoken "bot_token_telegram" //votre bot token telegram
const char *apiKey = "clef_api_openai"; //votre clef api openai
//NE PAS OUBLIER DE METTRE L'ID DE VOTRE ASSISTANT OPENAI PLUS BAS DANS >runAssistantOnThread

WiFiClientSecure telegramClient;
WiFiClientSecure openaiClient;
UniversalTelegramBot bot(BOTtoken, telegramClient);
String threadId;

const unsigned long BOT_MTBS = 1000; // temps entre chaque scan de message sur telegram 1 seconde
unsigned long bot_lasttime;

void setup() {
  Serial.begin(115200);
  WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK); //facultatif, indispensable sur ma freebox crystal
  WiFi.begin(ssid, password);
  telegramClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  openaiClient.setCACert(OPENAI_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connexion au WiFi en cours...");
  }
  Serial.println("Connecté au réseau WiFi");
}

String createThread() {
  Serial.println("createThread appelé"); //debug
  String apiUrl = "https://api.openai.com/v1/threads";
  String apiKeyHeader = "Bearer " + String(apiKey);
  HTTPClient http;
  http.begin(openaiClient, apiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", apiKeyHeader);
  http.addHeader("OpenAI-Beta", "assistants=v1");
  int httpResponseCode = http.POST("");
  String response = "";
  if (httpResponseCode == 200) {
    response = http.getString();
  } else {
    Serial.printf("Error %i: %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
  }
  http.end();
  DynamicJsonDocument jsonResponse(1024);
  deserializeJson(jsonResponse, response);
  Serial.println(jsonResponse["id"].as<String>()); //debug
  return jsonResponse["id"].as<String>();
}

void addMessageToThread(const String &threadId, const String &role, const String &content) {
  Serial.println("addMessageToThread appelé"); //debug
  String apiUrl = "https://api.openai.com/v1/threads/" + threadId + "/messages";
  String apiKeyHeader = "Bearer " + String(apiKey);
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["role"] = role;
  jsonDoc["content"] = content;
  String payload;
  serializeJson(jsonDoc, payload);
  HTTPClient http;
  http.begin(openaiClient, apiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", apiKeyHeader);
  http.addHeader("OpenAI-Beta", "assistants=v1");
  int httpResponseCode = http.POST(payload.c_str());
  if (httpResponseCode != 200) {
    Serial.printf("Error %i: %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
  }
  http.end();
  return httpResponseCode
}

String runAssistantOnThread(const String &threadId) {
  Serial.println("runAssistantOnThread appelé"); //debug
  String apiUrl = "https://api.openai.com/v1/threads/" + threadId + "/runs";
  String apiKeyHeader = "Bearer " + String(apiKey);
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["assistant_id"] = "mettre l'id de votre assistant openai ici"; // mettre l'id de votre assistant openai ici
  String payload;
  serializeJson(jsonDoc, payload);
  HTTPClient http;
  http.begin(openaiClient, apiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", apiKeyHeader);
  http.addHeader("OpenAI-Beta", "assistants=v1");
  int httpResponseCode = http.POST(payload.c_str());
  String response = "";
  if (httpResponseCode == 200) {
    response = http.getString();
  } else {
    Serial.printf("Error %i: %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
  }
  http.end();
  DynamicJsonDocument jsonResponse(1024);
  deserializeJson(jsonResponse, response);
  Serial.println(jsonResponse["id"].as<String>()); //debug
  return jsonResponse["id"].as<String>();
}

String getRunStatus(const String &threadId, const String &runId) {
  Serial.println("getRunStatus appelé"); //debug
  String apiUrl = "https://api.openai.com/v1/threads/" + threadId + "/runs/" + runId;
  String apiKeyHeader = "Bearer " + String(apiKey);
  HTTPClient http;
  http.begin(openaiClient, apiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", apiKeyHeader);
  http.addHeader("OpenAI-Beta", "assistants=v1");
  int httpResponseCode = http.GET();
  String response = "";
  if (httpResponseCode == 200) {
    response = http.getString();
  } else {
    Serial.printf("Error %i: %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
  }
  http.end();
  DynamicJsonDocument jsonResponse(1024);
  deserializeJson(jsonResponse, response);
  Serial.println(jsonResponse["status"].as<String>()); //debug
  return jsonResponse["status"].as<String>();
}

String getAssistantMessagesFromThread(const String &threadId) {
  String apiUrl = "https://api.openai.com/v1/threads/" + threadId + "/messages";
  String apiKeyHeader = "Bearer " + String(apiKey);
  HTTPClient http;
  http.begin(openaiClient, apiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", apiKeyHeader);
  http.addHeader("OpenAI-Beta", "assistants=v1");
  int httpResponseCode = http.GET();
  String response = "";
  if (httpResponseCode == 200) {
    response = http.getString();
  } else {
    Serial.printf("Error %i: %s\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
  }
  http.end();
  DynamicJsonDocument jsonResponse(1024);
  deserializeJson(jsonResponse, response);
  return jsonResponse["data"][0]["content"][0]["text"]["value"].as<String>();
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      threadId = createThread(); // crée un thread quand /start est reçu
      addMessageToThread(threadId, "user", "Salut !"); //personaliser le premier message envoyé
      String runId = runAssistantOnThread(threadId);
      while (getRunStatus(threadId, runId) != "completed") {
        bot.sendChatAction(chat_id, "typing");
        delay(1000); // attendre une seconde avant de vérifier le statut
      }
      String chatGPTResponse = getAssistantMessagesFromThread(threadId);
      bot.sendMessage(chat_id, chatGPTResponse);
    } else {
      if (addMessageToThread(threadId, "user", text) == 400) {
        threadId = createThread(); // recrée un nouveau thread si erreur 400
        addMessageToThread(threadId, "user", "Salut !"); //personaliser le premier message envoyé
      }
      else {
      String runId = runAssistantOnThread(threadId);
      while (getRunStatus(threadId, runId) != "completed") {
        delay(1000); // attendre une seconde avant de vérifier le statut
      }
      String chatGPTResponse = getAssistantMessagesFromThread(threadId);
      bot.sendMessage(chat_id, chatGPTResponse);
      }
    }
  }
}

void loop() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}