from flask import Flask
from app.api.auth import auth_bp
from app.api.clans import clans_bp
from app.api.map import map_bp
from app.api.oppenet import oppenet_bp
from app.api.clanswar import clanswar_bp
from app.api.user import user_bp
app = Flask(__name__)

# 注册认证相关的蓝图
app.register_blueprint(auth_bp, url_prefix='/api')
# 注册部落相关的蓝图
app.register_blueprint(clans_bp, url_prefix='/api')
# 注册地图相关的蓝图
app.register_blueprint(map_bp, url_prefix='/api')
# 注册对手相关的蓝图
app.register_blueprint(oppenet_bp, url_prefix='/api')
# 部落战
app.register_blueprint(clanswar_bp, url_prefix='/api')
# 用户
app.register_blueprint(user_bp, url_prefix='/api')

@app.route('/')
def hello():
    return "Hello, this is your backend service!"

if __name__ == '__main__':
    # 绑定到所有网络接口，确保可以通过局域网 IP 访问
    app.run(host='0.0.0.0', port=5000)
