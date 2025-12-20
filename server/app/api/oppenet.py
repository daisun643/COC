from flask import Blueprint, request, jsonify
from app.utils.Oppenet import get_random_opponent
import time
import random

oppenet_bp = Blueprint('oppenet', __name__)


@oppenet_bp.route('/oppenet/get', methods=['GET'])
def get_opponent():
    """
    获取随机对手接口
    GET 参数: user_id (用户ID)
    返回: JSON 格式 {
        "success": bool,
        "message": str,
        "opponent_id": int or None,
        "opponent_name": str or None,
        "map_data": dict or None
    }
    """
    user_id = request.args.get('user_id')
    # 随机sleep 3-5秒
    time.sleep(random.randint(3, 5))
    # 参数验证
    if not user_id:
        return jsonify({
            "success": False,
            "message": "用户ID不能为空",
            "opponent_id": None,
            "opponent_name": None,
            "map_data": None
        }), 400
    
    try:
        user_id = int(user_id)
    except ValueError:
        return jsonify({
            "success": False,
            "message": "用户ID必须是数字",
            "opponent_id": None,
            "opponent_name": None,
            "map_data": None
        }), 400
    
    # 调用获取对手逻辑
    success, message, opponent_id, opponent_name, map_data = get_random_opponent(user_id)
    
    return jsonify({
        "success": success,
        "message": message,
        "opponent_id": opponent_id,
        "opponent_name": opponent_name,
        "map_data": map_data
    }), 200 if success else 404
