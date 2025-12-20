from flask import Blueprint, request, jsonify
from app.utils.Clans import (
    create_clan,
    get_all_clans_info,
    get_clan_info,
    get_clan_members,
    get_clan_owner,
    join_clan,
    leave_clan,
    disband_clan,
    search_clans_by_name,
    get_clan_chat_messages,
    send_clan_chat_message
)

clans_bp = Blueprint('clans', __name__)


@clans_bp.route('/clans/create', methods=['GET'])
def create():
    """
    创建部落接口
    GET 参数: name (部落名称), owner_id (所有者用户ID)
    返回: JSON 格式 {"success": bool, "message": str, "clan_id": str or None}
    """
    name = request.args.get('name')
    owner_id = request.args.get('owner_id')
    
    # 参数验证
    if not name or not owner_id:
        return jsonify({
            "success": False,
            "message": "部落名称和所有者ID不能为空",
            "clan_id": None
        }), 400
    
    try:
        owner_id = int(owner_id)
    except ValueError:
        return jsonify({
            "success": False,
            "message": "所有者ID必须是数字",
            "clan_id": None
        }), 400
    
    # 调用创建逻辑
    success, message, clan_id = create_clan(name, owner_id)
    
    return jsonify({
        "success": success,
        "message": message,
        "clan_id": clan_id
    }), 200 if success else 400

# 获取所有部落的简要信息
@clans_bp.route('/clans/all-info', methods=['GET'])
def all_info():
    """
    获取所有部落的简要信息
    返回: JSON 格式 {"success": bool, "message": str, "data": list or None}
    """
    success, message, data = get_all_clans_info()
    return jsonify({
        "success": success,
        "message": message,
        "data": data
    }), 200 if success else 400


@clans_bp.route('/clans/search', methods=['GET'])
def search():
    """
    搜索部落接口
    GET 参数: name (部落名称关键词)
    返回: JSON 格式 {"success": bool, "message": str, "data": list or None}
    """
    name_keyword = request.args.get('name', '')
    
    # 调用搜索逻辑
    success, message, data = search_clans_by_name(name_keyword)
    
    return jsonify({
        "success": success,
        "message": message,
        "data": data
    }), 200 if success else 400


@clans_bp.route('/clans/info', methods=['GET'])
def info():
    """
    获取部落信息接口
    GET 参数: clan_id (部落ID)
    返回: JSON 格式 {"success": bool, "message": str, "data": dict or None}
    """
    clan_id = request.args.get('clan_id')
    
    # 参数验证
    if not clan_id:
        return jsonify({
            "success": False,
            "message": "部落ID不能为空",
            "data": None
        }), 400
    
    # 调用获取信息逻辑
    success, message, data = get_clan_info(clan_id)
    
    return jsonify({
        "success": success,
        "message": message,
        "data": data
    }), 200 if success else 404


@clans_bp.route('/clans/members', methods=['GET'])
def members():
    """
    获取部落成员接口
    GET 参数: clan_id (部落ID)
    返回: JSON 格式 {"success": bool, "message": str, "members": list or None}
    """
    clan_id = request.args.get('clan_id')
    
    # 参数验证
    if not clan_id:
        return jsonify({
            "success": False,
            "message": "部落ID不能为空",
            "members": None
        }), 400
    
    # 调用获取成员逻辑
    success, message, members = get_clan_members(clan_id)
    
    return jsonify({
        "success": success,
        "message": message,
        "members": members
    }), 200 if success else 404

# 我需要返回的是拥有者的id
@clans_bp.route('/clans/owner', methods=['GET'])
def owner():
    """
    获取部落所有者接口
    GET 参数: clan_id (部落ID)
    返回: JSON 格式 {"success": bool, "message": str, "owner_id": int or None}
    """
    clan_id = request.args.get('clan_id')
    
    # 参数验证
    if not clan_id:
        return jsonify({
            "success": False,
            "message": "部落ID不能为空",
            "owner_id": None
        }), 400
    
    # 调用获取所有者逻辑
    success, message, owner_id = get_clan_owner(clan_id)
    
    return jsonify({
        "success": success,
        "message": message,
        "owner_id": owner_id
    }), 200 if success else 404


@clans_bp.route('/clans/join', methods=['GET'])
def join():
    """
    用户加入部落接口
    GET 参数: clan_id (部落ID), user_id (用户ID)
    返回: JSON 格式 {"success": bool, "message": str}
    """
    clan_id = request.args.get('clan_id')
    user_id = request.args.get('user_id')
    
    # 参数验证
    if not clan_id or not user_id:
        return jsonify({
            "success": False,
            "message": "部落ID和用户ID不能为空"
        }), 400
    
    try:
        user_id = int(user_id)
    except ValueError:
        return jsonify({
            "success": False,
            "message": "用户ID必须是数字"
        }), 400
    
    # 调用加入逻辑
    success, message = join_clan(clan_id, user_id)
    
    return jsonify({
        "success": success,
        "message": message
    }), 200 if success else 400

# 增加解散功能
@clans_bp.route('/clans/disband', methods=['GET'])
def disband():
    """
    解散部落接口（只有所有者可以解散）
    GET 参数: clan_id (部落ID), user_id (用户ID)
    返回: JSON 格式 {"success": bool, "message": str}
    """
    clan_id = request.args.get('clan_id')
    user_id = request.args.get('user_id')
    
    # 参数验证
    if not clan_id or not user_id:
        return jsonify({
            "success": False,
            "message": "部落ID和用户ID不能为空"
        }), 400
    
    try:
        user_id = int(user_id)
    except ValueError:
        return jsonify({
            "success": False,
            "message": "用户ID必须是数字"
        }), 400
    
    # 调用解散逻辑
    success, message = disband_clan(clan_id, user_id)
    
    return jsonify({
        "success": success,
        "message": message
    }), 200 if success else 400


@clans_bp.route('/clans/leave', methods=['GET'])
def leave():
    """
    用户退出部落接口
    GET 参数: clan_id (部落ID), user_id (用户ID)
    返回: JSON 格式 {"success": bool, "message": str}
    """
    clan_id = request.args.get('clan_id')
    user_id = request.args.get('user_id')
    
    # 参数验证
    if not clan_id or not user_id:
        return jsonify({
            "success": False,
            "message": "部落ID和用户ID不能为空"
        }), 400
    
    try:
        user_id = int(user_id)
    except ValueError:
        return jsonify({
            "success": False,
            "message": "用户ID必须是数字"
        }), 400
    
    # 调用退出逻辑
    success, message = leave_clan(clan_id, user_id)
    
    return jsonify({
        "success": success,
        "message": message
    }), 200 if success else 400


@clans_bp.route('/clans/chat/messages', methods=['GET'])
def chat_messages():
    """
    获取部落聊天室消息接口
    GET 参数: clan_id (部落ID), limit (可选，返回条数限制)
    返回: JSON 格式 {"success": bool, "message": str, "count": int, "messages": list or None}
    """
    clan_id = request.args.get('clan_id')
    limit = request.args.get('limit')
    
    # 参数验证
    if not clan_id:
        return jsonify({
            "success": False,
            "message": "部落ID不能为空",
            "count": 0,
            "messages": None
        }), 400
    
    # 处理 limit 参数
    limit_int = None
    if limit:
        try:
            limit_int = int(limit)
            if limit_int < 0:
                limit_int = None
        except ValueError:
            limit_int = None
    
    # 调用获取消息逻辑
    success, message, messages, count = get_clan_chat_messages(clan_id, limit_int)
    
    return jsonify({
        "success": success,
        "message": message,
        "count": count,
        "messages": messages
    }), 200 if success else 404


@clans_bp.route('/clans/chat/send', methods=['GET'])
def chat_send():
    """
    发送部落聊天室消息接口
    GET 参数: clan_id (部落ID), user_id (用户ID), content (消息内容)
    返回: JSON 格式 {"success": bool, "message": str}
    """
    clan_id = request.args.get('clan_id')
    user_id = request.args.get('user_id')
    content = request.args.get('content')
    
    # 参数验证
    if not clan_id or not user_id or not content:
        return jsonify({
            "success": False,
            "message": "部落ID、用户ID和消息内容不能为空"
        }), 400
    
    try:
        user_id = int(user_id)
    except ValueError:
        return jsonify({
            "success": False,
            "message": "用户ID必须是数字"
        }), 400
    
    # 调用发送消息逻辑
    success, message = send_clan_chat_message(clan_id, user_id, content)
    
    return jsonify({
        "success": success,
        "message": message
    }), 200 if success else 400