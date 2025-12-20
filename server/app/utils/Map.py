import json
import os
from pathlib import Path

# 获取地图文件目录
BASE_DIR = Path(__file__).parent.parent.parent
MAP_DIR = BASE_DIR / 'static' / 'map'


def get_map_file_path(user_id):
    """获取用户地图文件路径"""
    os.makedirs(MAP_DIR, exist_ok=True)
    return MAP_DIR / f"{user_id}.json"


def load_user_map(user_id):
    """
    加载用户地图数据
    参数: user_id (用户ID)
    返回: (success: bool, message: str, map_data: dict or None)
    """
    map_file = get_map_file_path(user_id)
    
    if not os.path.exists(map_file):
        return False, "地图文件不存在", None
    
    try:
        with open(map_file, 'r', encoding='utf-8') as f:
            map_data = json.load(f)
            return True, "获取成功", map_data
    except (json.JSONDecodeError, IOError) as e:
        return False, f"读取地图文件失败: {str(e)}", None


def save_user_map(user_id, map_data):
    """
    保存用户地图数据
    参数: user_id (用户ID), map_data (地图数据字典)
    返回: (success: bool, message: str)
    """
    if not map_data:
        return False, "地图数据不能为空"
    
    map_file = get_map_file_path(user_id)
    
    try:
        os.makedirs(MAP_DIR, exist_ok=True)
        with open(map_file, 'w', encoding='utf-8') as f:
            json.dump(map_data, f, ensure_ascii=False, indent=4)
        return True, "保存成功"
    except (IOError, TypeError) as e:
        return False, f"保存地图文件失败: {str(e)}"

