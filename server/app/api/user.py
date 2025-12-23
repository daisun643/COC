from flask import Blueprint, request, jsonify
from app.utils.User import set_clanid, get_clainid

user_bp = Blueprint('user', __name__)


@user_bp.route('/set_clanid', methods=['GET'])
def api_set_clanid():
	"""
	设置用户的 clan_id
	GET 参数: id (用户ID), clan_id
	返回: JSON {"success": bool, "message": str}
	"""
	user_id = request.args.get('id')
	clan_id = request.args.get('clan_id')

	if not user_id or clan_id is None:
		return jsonify({"success": False, "message": "参数 id 和 clan_id 不能为空"}), 400

	success, message = set_clanid(user_id, clan_id)
	return jsonify({"success": success, "message": message}), 200 if success else 404


@user_bp.route('/get_clanid', methods=['GET'])
def api_get_clanid():
	"""
	获取用户的 clan_id
	GET 参数: id (用户ID)
	返回: JSON {"success": bool, "message": str, "clan_id": value or null}
	"""
	user_id = request.args.get('id')
	if not user_id:
		return jsonify({"success": False, "message": "参数 id 不能为空", "clan_id": None}), 400

	success, message, clan_id = get_clainid(user_id)
	return jsonify({"success": success, "message": message, "clan_id": clan_id}), 200 if success else 404
