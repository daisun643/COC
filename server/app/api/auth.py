from flask import Blueprint, request, jsonify
from app.utils.User import register_user, login_user

auth_bp = Blueprint('auth', __name__)


@auth_bp.route('/register', methods=['GET'])
def register():
    """
    注册接口
    GET 参数: name (用户名), password (密码)
    返回: JSON 格式 {"success": bool, "message": str, "user_id": int or None}
    """
    name = request.args.get('name')
    password = request.args.get('password')
    
    # 参数验证
    if not name or not password:
        return jsonify({
            "success": False,
            "message": "用户名和密码不能为空",
            "user_id": None
        }), 400
    
    # 调用注册逻辑
    success, message, user_id = register_user(name, password)
    
    return jsonify({
        "success": success,
        "message": message,
        "user_id": user_id
    }), 200 if success else 400


@auth_bp.route('/login', methods=['GET'])
def login():
    """
    登录接口
    GET 参数: id (用户ID), password (密码)
    返回: JSON 格式 {"success": bool, "message": str, "user_id": int or None}
    """
    id = request.args.get('id')
    id = int(id)
    password = request.args.get('password')
    
    # 参数验证
    if not id or not password:
        return jsonify({
            "success": False,
            "message": "用户ID和密码不能为空",
            "name": None
        }), 400
    
    # 调用登录逻辑
    success, message, name = login_user(id, password)
    
    return jsonify({
        "success": success,
        "message": message,
        "name": name
    }), 200 if success else 401

