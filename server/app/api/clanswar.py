from flask import Blueprint, request, jsonify
import json
from app.utils.ClansWar import (
	start_war,
	get_war_overview,
	get_war_map,
	update_war_map,
)


clanswar_bp = Blueprint('clanswar', __name__)


@clanswar_bp.route('/clanswar/start', methods=['GET'])
def start():
	"""开启部落战
	GET 参数: clans_id
	返回: {success, message, war_id, history_path}
	"""
	clans_id = request.args.get('clans_id')
	if not clans_id:
		return jsonify({"success": False, "message": "clans_id 不能为空", "war_id": None, "history_path": None}), 400

	success, message, war_id, history_path = start_war(clans_id)
	return jsonify({"success": success, "message": message, "war_id": war_id, "history_path": history_path}), 200 if success else 400


@clanswar_bp.route('/clanswar/overview', methods=['GET'])
def overview():
	"""获取部落战概览
	GET 参数: clans_id
	返回: {success, message, overview}
	"""
	clans_id = request.args.get('clans_id')
	if not clans_id:
		return jsonify({"success": False, "message": "clans_id 不能为空", "overview": None}), 400

	success, message, overview = get_war_overview(clans_id)
	return jsonify({"success": success, "message": message, "overview": overview}), 200 if success else 404


@clanswar_bp.route('/clanswar/map', methods=['GET'])
def get_map():
	"""获取部落战某张地图
	GET 参数: clans_id, map_id
	返回: {success, message, data}
	"""
	clans_id = request.args.get('clans_id')
	map_id = request.args.get('map_id')
	if not clans_id or not map_id:
		return jsonify({"success": False, "message": "clans_id 和 map_id 不能为空", "data": None}), 400

	success, message, data = get_war_map(clans_id, map_id)
	return jsonify({"success": success, "message": message, "data": data}), 200 if success else 404


@clanswar_bp.route('/clanswar/map/save', methods=['GET'])
def save_map():
	"""保存/更新部落战地图
	GET 参数: clans_id, map_id, map_data (JSON字符串)
	返回: {success, message}
	"""
	clans_id = request.args.get('clans_id')
	map_id = request.args.get('map_id')
	map_data_str = request.args.get('map_data')

	if not clans_id or not map_id or not map_data_str:
		return jsonify({"success": False, "message": "clans_id、map_id 和 map_data 不能为空"}), 400

	try:
		map_data = json.loads(map_data_str)
	except json.JSONDecodeError as e:
		return jsonify({"success": False, "message": f"地图数据格式错误: {e}"}), 400

	success, message = update_war_map(clans_id, map_id, map_data)
	return jsonify({"success": success, "message": message}), 200 if success else 400
