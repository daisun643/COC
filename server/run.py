from flask import Flask
from app.api.auth import auth_bp
from app.api.clans import clans_bp

app = Flask(__name__)

# 注册认证相关的蓝图
app.register_blueprint(auth_bp, url_prefix='/api')
# 注册部落相关的蓝图
app.register_blueprint(clans_bp, url_prefix='/api')

@app.route('/')
def hello():
    return "Hello, this is your backend service!"

if __name__ == '__main__':
    # 绑定到所有网络接口，确保可以通过局域网 IP 访问
    app.run(host='127.0.0.1', port=5000)
