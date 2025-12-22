# 这里需要一系列支持部落战的工具函数

import json
import os
import random
import shutil
from datetime import datetime, timedelta
from pathlib import Path

TEMPLATE_COUNT = 3

# 与其他 utils 保持一致的 BASE_DIR 和文件路径常量
BASE_DIR = Path(__file__).parent.parent.parent
WAR_DIR = BASE_DIR / 'static' / 'clans' / 'war'
SUMMARY_FILE = WAR_DIR / 'summary.json'
TEMPLATE_DIR = WAR_DIR / 'template'
HISTORY_DIR = WAR_DIR / 'history'


def load_summary():
	"""加载 summary.json，若不存在则创建默认结构并返回字典。"""
	try:
		os.makedirs(WAR_DIR, exist_ok=True)
		if not SUMMARY_FILE.exists():
			default = {"new-war-id": 0, "clans-id": {}}
			with open(SUMMARY_FILE, 'w', encoding='utf-8') as f:
				json.dump(default, f, ensure_ascii=False, indent=4)
			return default

		with open(SUMMARY_FILE, 'r', encoding='utf-8') as f:
			return json.load(f)
	except (json.JSONDecodeError, IOError):
		return {"new-war-id": 0, "clans-id": {}}


def save_summary(data):
	"""保存 summary.json（覆盖）。"""
	try:
		os.makedirs(SUMMARY_FILE.parent, exist_ok=True)
		tmp = str(SUMMARY_FILE) + '.tmp'
		with open(tmp, 'w', encoding='utf-8') as f:
			json.dump(data, f, ensure_ascii=False, indent=4)
		os.replace(tmp, str(SUMMARY_FILE))
		return True
	except (IOError, TypeError) as e:
		return False


def start_war(clans_id):
	"""开启部落战。

	返回: (success: bool, message: str, war_id: str or None, history_path: str or None)
	"""
	data = load_summary()

	try:
		new_id = int(data.get('new-war-id', 0)) + 1
	except (TypeError, ValueError):
		new_id = 1

	war_id = f"war-{new_id}"

	# 更新 summary
	data['new-war-id'] = new_id
	if 'clans-id' not in data or not isinstance(data['clans-id'], dict):
		data['clans-id'] = {}

	timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
	data['clans-id'][str(clans_id)] = {
		'start-time': timestamp,
		'war-id': war_id,
	}

	# 选择模板并复制到 history
	template_id = random.randint(1, TEMPLATE_COUNT)
	src = TEMPLATE_DIR / str(template_id)
	dest = HISTORY_DIR / war_id

	if not src.exists() or not src.is_dir():
		return False, f"模板不存在: {src}", None, None

	if dest.exists():
		return False, f"目标历史目录已存在: {dest}", None, None

	try:
		shutil.copytree(str(src), str(dest))
	except Exception as e:
		return False, f"复制模板失败: {e}", None, None

	# 保存 summary
	if not save_summary(data):
		return False, "保存 summary.json 失败", None, None

	return True, "开启部落战成功", war_id, str(dest)
 

def get_war_overview(clans_id):
	"""获取部落战概览。

	返回: (success: bool, message: str, overview: dict or None)
	overview 格式: {"map_id": {"stars": int, "cnt": int}, ...}
	只有当部落战存在且未结束(开始时间 + 2天)时返回数据。
	"""
	data = load_summary()

	entry = data.get('clans-id', {}).get(str(clans_id))
	if not entry:
		return False, "未开始部落战", None

	start_str = entry.get('start-time')
	if not start_str:
		return False, "开始时间未记录", None

	try:
		start_dt = datetime.strptime(start_str, '%Y-%m-%d %H:%M:%S')
	except Exception:
		return False, "开始时间格式错误", None

	# 判断是否超过 2 天
	if datetime.now() > (start_dt + timedelta(days=2)):
		return False, "部落战已结束", None

	war_id = entry.get('war-id')
	if not war_id:
		return False, "war-id 未记录", None

	war_dir = HISTORY_DIR / war_id
	if not war_dir.exists() or not war_dir.is_dir():
		return False, "历史目录不存在", None

	overview = {}
	for f in os.listdir(str(war_dir)):
		if not f.lower().endswith('.json'):
			continue
		map_id = os.path.splitext(f)[0]
		path = war_dir / f
		try:
			with open(path, 'r', encoding='utf-8') as fh:
				m = json.load(fh)
				stars = int(m.get('stars', 0)) if isinstance(m.get('stars', 0), (int, float, str)) else 0
				cnt = int(m.get('cnt', 0)) if isinstance(m.get('cnt', 0), (int, float, str)) else 0
				overview[map_id] = {'stars': stars, 'cnt': cnt}
		except Exception:
			overview[map_id] = {'stars': 0, 'cnt': 0}

	return True, "获取成功", overview


def get_war_map(clans_id, map_id):
	"""获取指定部落战地图文件的内容。

	返回: (success: bool, message: str, map_data: dict or None)
	"""
	data = load_summary()
	entry = data.get('clans-id', {}).get(str(clans_id))
	if not entry:
		return False, "未开始部落战", None

	war_id = entry.get('war-id')
	if not war_id:
		return False, "war-id 未记录", None

	map_file = HISTORY_DIR / war_id / f"{map_id}.json"
	if not map_file.exists():
		return False, "地图文件不存在", None

	try:
		with open(map_file, 'r', encoding='utf-8') as f:
			return True, "获取成功", json.load(f)
	except Exception as e:
		return False, f"读取地图失败: {e}", None


def update_war_map(clans_id, map_id, map_data):
	"""更新部落战中的某张地图（覆盖写入）。

	返回: (success: bool, message: str)
	"""
	data = load_summary()
	entry = data.get('clans-id', {}).get(str(clans_id))
	if not entry:
		return False, "未开始部落战"

	war_id = entry.get('war-id')
	if not war_id:
		return False, "war-id 未记录"

	war_path = HISTORY_DIR / war_id
	os.makedirs(war_path, exist_ok=True)
	map_file = war_path / f"{map_id}.json"

	try:
		try:
			with open(map_file, 'r', encoding='utf-8') as f:
				existing_data = json.load(f)
			cnt = existing_data.get('cnt', 0)  # 如果没有 cnt，使用默认值 0
		except FileNotFoundError:
			cnt = 0  # 如果文件不存在，初始化 cnt 为 0

		new_data = map_data
		new_data['cnt'] = cnt - 1  # 更新 cnt 值
		# 将新的数据写入文件
		with open(map_file, 'w', encoding='utf-8') as f:
			json.dump(new_data, f, ensure_ascii=False, indent=4)

		return True, "保存成功"
	except Exception as e:
		return False, f"保存失败: {e}"