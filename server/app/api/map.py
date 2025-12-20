from flask import Blueprint, request, jsonify
import json
from app.utils.Map import load_user_map, save_user_map

map_bp = Blueprint('map', __name__)


@map_bp.route('/map/get', methods=['GET'])
def get_map():
    """
    获取用户地图接口
    GET 参数: user_id (用户ID)
    返回: JSON 格式 {"success": bool, "message": str, "data": dict or None}
    """
    user_id = request.args.get('user_id')
    
    # 参数验证
    if not user_id:
        return jsonify({
            "success": False,
            "message": "用户ID不能为空",
            "data": None
        }), 400
    
    try:
        user_id = int(user_id)
    except ValueError:
        return jsonify({
            "success": False,
            "message": "用户ID必须是数字",
            "data": None
        }), 400
    
    # 调用获取地图逻辑
    success, message, map_data = load_user_map(str(user_id))
    
    return jsonify({
        "success": success,
        "message": message,
        "data": map_data
    }), 200 if success else 404


@map_bp.route('/map/save', methods=['GET'])
def save_map():
    """
    保存用户地图接口
    GET 参数: user_id (用户ID), map_data (地图数据，JSON字符串)
    返回: JSON 格式 {"success": bool, "message": str}
    """
    user_id = request.args.get('user_id')
    map_data_str = request.args.get('map_data')
    
    # 参数验证
    if not user_id or not map_data_str:
        return jsonify({
            "success": False,
            "message": "用户ID和地图数据不能为空"
        }), 400
    
    try:
        user_id = int(user_id)
    except ValueError:
        return jsonify({
            "success": False,
            "message": "用户ID必须是数字"
        }), 400
    
    # 解析地图数据JSON字符串
    try:
        map_data = json.loads(map_data_str)
    except json.JSONDecodeError as e:
        return jsonify({
            "success": False,
            "message": f"地图数据格式错误: {str(e)}"
        }), 400
    
    # 调用保存地图逻辑
    success, message = save_user_map(str(user_id), map_data)
    
    return jsonify({
        "success": success,
        "message": message
    }), 200 if success else 400
