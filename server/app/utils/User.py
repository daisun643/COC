import json
import os
from pathlib import Path

# 获取 users.json 的路径
BASE_DIR = Path(__file__).parent.parent.parent
USERS_FILE = BASE_DIR / 'static' / 'user' / 'users.json'


def load_users():
    """加载所有用户数据"""
    if not os.path.exists(USERS_FILE):
        # 如果文件不存在，创建默认结构
        with open(USERS_FILE, 'w', encoding='utf-8') as f:
            json.dump({"users": []}, f, ensure_ascii=False, indent=4)
        return {"users": []}
    
    try:
        with open(USERS_FILE, 'r', encoding='utf-8') as f:
            return json.load(f)
    except (json.JSONDecodeError, IOError):
        return {"users": []}


def save_users(users_data):
    """保存用户数据到文件"""
    os.makedirs(USERS_FILE.parent, exist_ok=True)
    with open(USERS_FILE, 'w', encoding='utf-8') as f:
        json.dump(users_data, f, ensure_ascii=False, indent=4)


def register_user(name, password):
    """
    注册新用户
    返回: (success: bool, message: str, user_id: int or None)
    """
    users_data = load_users()
    
    # 检查用户名是否已存在
    for user in users_data.get("users", []):
        if user.get("name") == name:
            return False, "用户名已存在", None
    
    # 生成新用户ID
    if users_data.get("users"):
        new_id = max(user.get("id", 0) for user in users_data["users"]) + 1
    else:
        new_id = 1
    
    # 添加新用户
    new_user = {
        "id": new_id,
        "name": name,
        "password": password
    }
    users_data["users"].append(new_user)
    save_users(users_data)
    
    return True, "注册成功", new_id


def login_user(id, password):
    """
    用户登录验证
    返回: (success: bool, message: str, user_id: int or None)
    """
    users_data = load_users()
    
    # 查找用户
    for user in users_data.get("users", []):
        if user.get("id") == id:
            if user.get("password") == password:
                return True, "登录成功", user.get("name")
            else:
                return False, "密码错误", None
    
    return False, "用户不存在", None

def set_clanid(user_id, clain_id):
    """设置用户的 clan_id。

    返回: (success: bool, message: str)
    """
    users_data = load_users()
    for user in users_data.get("users", []):
        try:
            if int(user.get("id", 0)) == int(user_id):
                user["clan_id"] = clain_id
                save_users(users_data)
                return True, "clan_id 已设置"
        except (TypeError, ValueError):
            continue
    return False, "用户不存在"

def get_clainid(user_id):
    """获取用户的 clan_id。

    返回: (success: bool, message: str, clan_id or None)
    """
    users_data = load_users()
    for user in users_data.get("users", []):
        try:
            if int(user.get("id", 0)) == int(user_id):
                return True, "获取成功", user.get("clan_id")
        except (TypeError, ValueError):
            continue
    return False, "用户不存在", None