import json
import os
from pathlib import Path

# 获取 clans.json 的路径
BASE_DIR = Path(__file__).parent.parent.parent
CLANS_FILE = BASE_DIR / 'static' / 'clans' / 'clans.json'
CHAT_FILE = BASE_DIR / 'static' / 'clans' / 'chat.json'


def load_clans():
    """加载所有部落数据"""
    if not os.path.exists(CLANS_FILE):
        # 如果文件不存在，创建默认结构
        os.makedirs(CLANS_FILE.parent, exist_ok=True)
        with open(CLANS_FILE, 'w', encoding='utf-8') as f:
            json.dump({}, f, ensure_ascii=False, indent=4)
        return {}
    
    try:
        with open(CLANS_FILE, 'r', encoding='utf-8') as f:
            return json.load(f)
    except (json.JSONDecodeError, IOError):
        return {}


def save_clans(clans_data):
    """保存部落数据到文件"""
    os.makedirs(CLANS_FILE.parent, exist_ok=True)
    with open(CLANS_FILE, 'w', encoding='utf-8') as f:
        json.dump(clans_data, f, ensure_ascii=False, indent=4)


def create_clan(name, owner_id):
    """
    创建部落
    参数: name (部落名称), owner_id (所有者用户ID)
    返回: (success: bool, message: str, clan_id: str or None)
    """
    clans_data = load_clans()
    
    # 检查部落名称是否已存在
    for clan_id, clan_info in clans_data.items():
        if clan_info.get("name") == name:
            return False, "部落名称已存在", None
    
    # 生成新部落ID
    if clans_data:
        # 找到最大的数字ID
        numeric_ids = [int(k) for k in clans_data.keys() if k.isdigit()]
        new_id = str(max(numeric_ids) + 1) if numeric_ids else "1"
    else:
        new_id = "1"
    
    # 创建新部落
    new_clan = {
        "name": name,
        "owner": owner_id,
        "members": [owner_id]  # 所有者自动成为成员
    }
    clans_data[new_id] = new_clan
    save_clans(clans_data)
    
    return True, "创建部落成功", new_id


def get_all_clans_info():
    """
    获取所有部落的信息（名字、成员数量）
    返回: (success: bool, message: str, data: list)
    """
    clans_data = load_clans()
    info = []
    for clan_id, clan_info in clans_data.items():
        info.append({
            "id": str(clan_id),  # 添加部落ID
            "name": clan_info.get("name"),
            "member_count": len(clan_info.get("members", []))
        })
    return True, "获取成功", info


def search_clans_by_name(name_keyword):
    """
    按名称搜索部落
    参数: name_keyword (部落名称关键词)
    返回: (success: bool, message: str, data: list)
    """
    clans_data = load_clans()
    info = []
    keyword_lower = name_keyword.lower() if name_keyword else ""
    
    for clan_id, clan_info in clans_data.items():
        clan_name = clan_info.get("name", "")
        # 模糊匹配：如果部落名称包含关键词
        if keyword_lower in clan_name.lower():
            info.append({
                "id": str(clan_id),  # 添加部落ID
                "name": clan_name,
                "member_count": len(clan_info.get("members", []))
            })
    
    return True, "搜索成功", info

def get_clan_info(clan_id):
    """
    获取部落信息（名字、成员数量）
    参数: clan_id (部落ID)
    返回: (success: bool, message: str, data: dict or None)
    """
    clans_data = load_clans()
    
    if clan_id not in clans_data:
        return False, "部落不存在", None
    
    clan = clans_data[clan_id]
    info = {
        "name": clan.get("name"),
        "member_count": len(clan.get("members", []))
    }
    
    return True, "获取成功", info


def get_clan_members(clan_id):
    """
    获取部落成员（用户名）
    参数: clan_id (部落ID)
    返回: (success: bool, message: str, members: list or None)
    """
    from app.utils.User import load_users
    
    clans_data = load_clans()
    
    if clan_id not in clans_data:
        return False, "部落不存在", None
    
    clan = clans_data[clan_id]
    member_ids = clan.get("members", [])
    
    # 加载用户数据以获取用户名
    users_data = load_users()
    user_dict = {user.get("id"): user.get("name") for user in users_data.get("users", [])}
    
    # 将用户ID转换为用户名（处理整数和字符串类型）
    member_names = []
    for member_id in member_ids:
        # 确保 member_id 是整数类型以便比较
        if isinstance(member_id, str) and member_id.isdigit():
            member_id = int(member_id)
        member_name = user_dict.get(member_id, f"用户{member_id}")
        member_names.append(member_name)
    
    return True, "获取成功", member_names


def get_clan_owner(clan_id):
    """
    获取部落所有者（用户名）
    参数: clan_id (部落ID)
    返回: (success: bool, message: str, owner_name: str or None)
    """
    from app.utils.User import load_users
    
    clans_data = load_clans()
    
    if clan_id not in clans_data:
        return False, "部落不存在", None
    
    owner_id = clans_data[clan_id].get("owner")
    # 确保 owner_id 是整数类型以便比较
    if isinstance(owner_id, str) and owner_id.isdigit():
        owner_id = int(owner_id)
    
    # 加载用户数据以获取用户名
    users_data = load_users()
    for user in users_data.get("users", []):
        if user.get("id") == owner_id:
            return True, "获取成功", user.get("name")
    
    return False, "所有者不存在", None


def join_clan(clan_id, user_id):
    """
    用户加入部落
    参数: clan_id (部落ID), user_id (用户ID)
    返回: (success: bool, message: str)
    """
    clans_data = load_clans()
    
    if clan_id not in clans_data:
        return False, "部落不存在"
    
    clan = clans_data[clan_id]
    members = clan.get("members", [])
    
    # 检查用户是否已在部落中（处理整数和字符串类型）
    for member in members:
        if member == user_id or (isinstance(member, str) and member.isdigit() and int(member) == user_id) or (isinstance(user_id, str) and user_id.isdigit() and int(user_id) == member):
            return False, "用户已在部落中"
    
    # 添加用户到部落
    members.append(user_id)
    clan["members"] = members
    clans_data[clan_id] = clan
    save_clans(clans_data)
    
    return True, "加入部落成功"


def leave_clan(clan_id, user_id):
    """
    用户退出部落
    参数: clan_id (部落ID), user_id (用户ID)
    返回: (success: bool, message: str)
    """
    clans_data = load_clans()
    
    if clan_id not in clans_data:
        return False, "部落不存在"
    
    clan = clans_data[clan_id]
    members = clan.get("members", [])
    
    # 检查是否是所有者
    owner_id = clan.get("owner")
    if isinstance(owner_id, str) and owner_id.isdigit():
        owner_id = int(owner_id)
    if owner_id == user_id:
        return False, "所有者不能退出部落"
    
    # 检查用户是否在部落中并移除（处理整数和字符串类型）
    member_to_remove = None
    for member in members:
        if member == user_id or (isinstance(member, str) and member.isdigit() and int(member) == user_id) or (isinstance(user_id, str) and user_id.isdigit() and int(user_id) == member):
            member_to_remove = member
            break
    
    if member_to_remove is None:
        return False, "用户不在部落中"
    
    # 从部落中移除用户
    members.remove(member_to_remove)
    clan["members"] = members
    clans_data[clan_id] = clan
    save_clans(clans_data)
    
    return True, "退出部落成功"


def load_chat():
    """加载所有聊天数据"""
    if not os.path.exists(CHAT_FILE):
        # 如果文件不存在，创建默认结构
        os.makedirs(CHAT_FILE.parent, exist_ok=True)
        with open(CHAT_FILE, 'w', encoding='utf-8') as f:
            json.dump({}, f, ensure_ascii=False, indent=4)
        return {}
    
    try:
        with open(CHAT_FILE, 'r', encoding='utf-8') as f:
            return json.load(f)
    except (json.JSONDecodeError, IOError):
        return {}


def save_chat(chat_data):
    """保存聊天数据到文件"""
    os.makedirs(CHAT_FILE.parent, exist_ok=True)
    with open(CHAT_FILE, 'w', encoding='utf-8') as f:
        json.dump(chat_data, f, ensure_ascii=False, indent=4)


def get_clan_chat_messages(clan_id, limit=None):
    """
    获取部落聊天室消息
    参数: clan_id (部落ID), limit (可选，返回条数限制)
    返回: (success: bool, message: str, messages: list or None, count: int)
    """
    chat_data = load_chat()
    
    # 检查部落是否存在
    clans_data = load_clans()
    if clan_id not in clans_data:
        return False, "部落不存在", None, 0
    
    # 获取该部落的聊天消息
    clan_chat = chat_data.get(str(clan_id), {})
    messages = clan_chat.get("messages", [])
    # 这里需要处理为空的情况
    if not messages:
        return True, "获取成功", [], 0
    # 记录总消息数
    total_count = len(messages)
    
    # 加载用户数据以获取用户名
    from app.utils.User import load_users
    users_data = load_users()
    user_dict = {user.get("id"): user.get("name") for user in users_data.get("users", [])}
    
    # 按时间降序排序（最新的在前）
    # 假设时间格式为 "YYYY-MM-DD HH:MM:SS"
    messages_sorted = sorted(messages, key=lambda x: x.get("time", ""), reverse=True)
    
    # 将 sender (user-id) 转换为用户名
    for message in messages_sorted:
        sender_id = message.get("sender", "")
        # 处理整数和字符串类型
        if isinstance(sender_id, str) and sender_id.isdigit():
            sender_id = int(sender_id)
        # 获取用户名，如果找不到则使用默认格式
        sender_name = user_dict.get(sender_id, f"用户{sender_id}")
        # 更新消息中的 sender 字段为用户名
        message["sender"] = sender_name
    
    # 如果指定了限制，只返回前 limit 条
    if limit is not None and limit > 0:
        messages_sorted = messages_sorted[:limit]
    messages_sorted = messages_sorted[::-1]
    return True, "获取成功", messages_sorted, total_count


def send_clan_chat_message(clan_id, user_id, content):
    """
    发送部落聊天室消息
    参数: clan_id (部落ID), user_id (用户ID), content (消息内容)
    返回: (success: bool, message: str)
    """
    # 检查部落是否存在
    clans_data = load_clans()
    if clan_id not in clans_data:
        return False, "部落不存在"
    
    # 检查用户是否在部落中
    clan = clans_data[clan_id]
    members = clan.get("members", [])
    
    # 处理整数和字符串类型
    user_in_clan = False
    for member in members:
        if member == user_id or (isinstance(member, str) and member.isdigit() and int(member) == user_id) or (isinstance(user_id, str) and user_id.isdigit() and int(user_id) == member):
            user_in_clan = True
            break
    
    if not user_in_clan:
        return False, "用户不在部落中"
    
    # 验证消息内容
    if not content or not content.strip():
        return False, "消息内容不能为空"
    
    # 加载聊天数据
    chat_data = load_chat()
    
    # 获取当前时间
    from datetime import datetime
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    # 创建消息对象
    message = {
        "sender": str(user_id),
        "content": content.strip(),
        "time": current_time
    }
    
    # 添加到部落的聊天记录中
    clan_id_str = str(clan_id)
    if clan_id_str not in chat_data:
        chat_data[clan_id_str] = {"messages": []}
    
    chat_data[clan_id_str]["messages"].append(message)
    
    # 保存聊天数据
    save_chat(chat_data)
    
    return True, "发送成功"
