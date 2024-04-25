#include <esp_task_wdt.h>
#include <WebServer.h>
#include <Update.h>
#include "solar.h"
#include "actor.h"

WebServer server(80);

uint16_t getWriteSuccessInfluxDB();

/* Style */
String style =
"<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
"#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* Login page */
String loginIndex =
"<meta charset=\"UTF-8\">"
"<form name=loginForm>"
"<h1>Ovládání okna</h1>"
"<input name=userid placeholder='Jméno'> "
"<input name=pwd placeholder=Heslo type=Password> "
"<input type=submit onclick=check(this.form) class=btn value=Přihlásit></form>"
"<script>"
"function check(form) {"
"if(form.userid.value=='admin' && form.pwd.value=='admin')"
"{window.open('/update')}"
"else"
"{alert('Chybné heslo')}"
"}"
"</script>" + style;

/* Server Index Page */
String serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"<label id='file-input' for='file'>   Choose file...</label>"
"<input type='submit' class=btn value='Update'>"
"<br><br>"
"<div id='prg'></div>"
"<br><div id='prgbar'><div id='bar'></div></div><br>"
"<input type=submit onclick=window.open('/status') class=btn value=Status>"
"<input type=submit onclick=window.open('/reset') class=btn value=Reboot></form>"
"<script>"
"function sub(obj){"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"};"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
"$.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"$('#bar').css('width',Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!') "
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>" + style;

String respClose =
"<script> setTimeout(function() {window.close()}, 1000);</script> OK";

String statusIndex() {
  String temp;

  String status;
  if (actor.windowMoveTime == 0) {
    if (actor.windowCurrentState == 100)
      status = " - opened";
    if (actor.windowCurrentState == 0)
      status = " - closed";
  } else
    status = " - moving";

  time_t now = time(nullptr);
  return "<!DOCTYPE HTML>"
         "<meta charset=\"UTF-8\">"
         "<html>"
         "<form>"
         "<h1>General</h1>"
         "Version: " VERSION " " __DATE__ " " __TIME__
         "<br/>Time: " + String(ctime(&now)) +
         "<br/>WiFi: " + WiFi.SSID() + " writeSucess: " + String( getWriteSuccessInfluxDB()) +
         "<br/>Uptime: " + String( millis() / 1000 / 3600) + "h " + String(( millis() / 1000) % 3600) + "s" +
         "<br/>Memory: " + String( ESP.getFreeHeap()) +
         "<br/>VCC: " + String( sensors.getV( sensors.sum.VCC), 2) + "V" +
         "<h1>Window control</h1>"
         "Current window: " + String( actor.windowCurrentState) + status +
         "<br/>Target window: " + String( actor.windowTargetState) +
         "<br/>Time to target: " + (actor.windowMoveTime > millis() ? String(( actor.windowMoveTime - millis()) / 1000) : "-") +
         "<br/><br/>Relays: " + String(actor.getActorRelay(), BIN) +
         "<br/>Actor Voltage/Current: " + String( sensors.getV( sensors.sum.actorVoltage), 2) + "V " + String( sensors.getV( sensors.sum.actorCurrent), 2) + "A"
         "<br/>Mode: " + (actor.windowAuto ? "auto " : "manual ") + String(sensors.getGreenhouseTemp()) + "°C " + (sensors.isRaining() ? "rain " : "-") + (sensors.strongWind() ? "wind" : "-") +
         "<h1>Sensors</h1>"
         "Temp min: " + String( MIN_TEMPERATURE) + "°C max: " + String( MAX_TEMPERATURE) + "°C<br/>" +
         owSensors.getOWStrWeb() +
         "<br/>Wind: " + weather.getWindDirStr(sensors.getV( sensors.sum.windDir)) + " " + String(sensors.getV( sensors.sum.windSpeed),1) + " km/h " + " Rain: " + String(sensors.getV( sensors.sum.rain),2) + " mm" +
         "<br/>SHTC3: " + String(sensors.getV( sensors.sum.shtTemp)) + "°C " + String(sensors.getV( sensors.sum.shtHum)) + "%" +
         "<br/>Light: " + String(sensors.getV( sensors.sum.lux)) + " lux" +
         "<br/>SPL06: " + String(sensors.getV( sensors.sum.SPL06Temp)) + "°C " + String(sensors.getV( sensors.sum.SPL06Press)) + " hPa" +
         "<br/><input type=submit onclick=window.open('/auto') class=btn value=Auto>"
         "<input type=submit onclick=window.open('/open') class=btn value=Open>"
         "<input type=submit onclick=window.open('/close') class=btn value=Close></form>"
         "</html>" + style;
}

void setupWebServer() {
/*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/status", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Refresh", "1");
    server.send(200, "text/html", statusIndex());
  });
  server.on("/auto", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    actor.windowAuto = true;
    server.send(200, "text/html", respClose);
  });
  server.on("/open", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    actor.windowAuto = false;
    actor.setWindow(actor.windowOpen);
    server.send(200, "text/html", respClose);
  });
  server.on("/close", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    actor.windowAuto = false;
    actor.setWindow(actor.windowClose);
    server.send(200, "text/html", respClose);
  });
  server.on("/reset", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", respClose);
    ESP.restart();
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.println("Update: " + upload.filename);
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      esp_task_wdt_reset();
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      esp_task_wdt_reset();
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}


void loopWebServer() {
  server.handleClient();
}