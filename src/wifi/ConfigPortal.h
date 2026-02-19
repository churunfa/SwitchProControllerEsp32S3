#ifndef SWITCHPROCONTROLLERESP32S3_CONFIGPORTAL_H
#define SWITCHPROCONTROLLERESP32S3_CONFIGPORTAL_H

#include <WiFi.h>
#include <WebServer.h>
#include <functional>

class ConfigPortal {
private:
    WebServer server;
    IPAddress apIP;
    std::function<void(String)> _onSaveCallback;

    const char* INDEX_HTML = R"=====(
<!DOCTYPE html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body { font-family: -apple-system, sans-serif; margin: 20px; background: #f4f4f9; color: #333; }
  .card { background: white; padding: 25px; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); max-width: 400px; margin: auto; }
  h3 { margin-top: 0; color: #007aff; text-align: center; }
  textarea { width: 100%; height: 180px; margin: 15px 0; border: 1px solid #ddd; border-radius: 8px; padding: 10px; font-family: monospace; box-sizing: border-box; font-size: 14px; }
  button { background: #007aff; color: white; border: none; padding: 15px; width: 100%; border-radius: 10px; font-size: 16px; font-weight: bold; cursor: pointer; }
  button:active { background: #0056b3; }
  .info { font-size: 12px; color: #666; background: #eee; padding: 10px; border-radius: 6px; margin-bottom: 15px; }
</style>
</head><body>
  <div class="card">
    <h3>Switch Pro 核心配置</h3>
    <div class="info">提示：若无法访问，请检查是否已关闭【移动数据/蜂窝网络】。</div>
    <textarea id="json">{"cmd":"update", "params":{}}</textarea>
    <button onclick="send()">保存并下发配置</button>
  </div>
  <script>
    function send() {
      const btn = document.querySelector('button');
      const content = document.getElementById('json').value;
      btn.disabled = true;
      btn.innerText = '正在发送...';
      fetch('/save', { method: 'POST', body: content })
      .then(res => {
        if(res.ok) alert('配置发送成功！');
        else alert('服务器响应错误');
      })
      .catch(err => alert('连接失败: ' + err))
      .finally(() => {
        btn.disabled = false;
        btn.innerText = '保存并下发配置';
      });
    }
  </script>
</body></html>)=====";

public:
    // 改用标准的局域网 IP
    ConfigPortal() : server(80), apIP(192, 168, 4, 1) {}

    void begin(const char* apName, std::function<void(String)> onSave) {
        _onSaveCallback = onSave;

        WiFi.mode(WIFI_AP);
        IPAddress gateway(192, 168, 4, 1);
        IPAddress subnet(255, 255, 255, 0);
        WiFi.softAPConfig(apIP, gateway, subnet);
        WiFi.softAP(apName);

        server.on("/", [this]() {
            server.send(200, "text/html", INDEX_HTML);
        });

        server.on("/save", HTTP_POST, [this]() {
            if (server.hasArg("plain")) {
                String data = server.arg("plain");
                server.send(200, "text/plain", "OK");
                if (_onSaveCallback) _onSaveCallback(data);
            }
        });

        server.begin();
        logPrintf("Web Server 已启动。访问地址: http://192.168.4.1\n");
    }

    void tick() {
        server.handleClient();
    }
};

#endif