import random
from app.utils.User import load_users
from app.utils.Map import load_user_map


def get_random_opponent(user_id):
    """
    随机选择一个对手（排除自己）
    参数: user_id (用户ID，整数)
    返回: (success: bool, message: str, opponent_id: int or None, opponent_name: str or None, map_data: dict or None)
    """
    users_data = load_users()
    users = users_data.get("users", [])
    
    # 排除自己，获取其他用户列表
    available_opponents = [user for user in users if user.get("id") != user_id]
    
    if not available_opponents:
        return False, "没有可用的对手", None, None, None
    
    # 随机选择一个对手
    opponent = random.choice(available_opponents)
    opponent_id = opponent.get("id")
    opponent_name = opponent.get("name", "")
    
    # 获取对手的地图数据
    success, message, map_data = load_user_map(str(opponent_id))
    
    if not success:
        # 如果地图文件不存在，返回空地图数据
        map_data = {}
    
    return True, "获取对手成功", opponent_id, opponent_name, map_data

