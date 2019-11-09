extends Node

class_name Client

var _ws = WebSocketClient.new()
var _write_mode = WebSocketPeer.WRITE_MODE_TEXT
var last_connected_client = 0

func _init():
	_ws.connect("connection_established", self, "_client_connected")
	_ws.connect("connection_error", self, "_client_disconnected")
	_ws.connect("connection_closed", self, "_client_disconnected")
	_ws.connect("server_close_request", self, "_client_close_request")
	_ws.connect("data_received", self, "_client_received")
	_ws.connect("peer_packet", self, "_client_received")
	_ws.connect("peer_connected", self, "_peer_connected")
	_ws.connect("connection_succeeded", self, "_client_connected", ["multiplayer_protocol"])
	_ws.connect("connection_failed", self, "_client_disconnected")
	_ws.verify_ssl = true


func _client_close_request(code, reason):
	Utils.print_log("Close code: %d, reason: %s" % [code, reason])


func _peer_connected(id):
	Utils.print_log("%s: Client just connected" % id)
	last_connected_client = id


func _exit_tree():
	_ws.disconnect_from_host(1001, "")


func _process(delta):
	if _ws.get_connection_status() == WebSocketClient.CONNECTION_DISCONNECTED:
		return
	_ws.poll()


func _client_connected(protocol):
	Utils.print_log( "Client just connected with protocol: %s" % protocol)
	_ws.get_peer(1).set_write_mode(_write_mode)


func _client_disconnected(clean=true):
	Utils.print_log( "Client just disconnected. Was clean: %s" % clean)


func _client_received(p_id = 1):
	var packet = _ws.get_peer(1).get_packet()
	var is_string = _ws.get_peer(1).was_string_packet()
	Utils.print_log( "Received data. BINARY: %s: %s" % [not is_string, Utils.decode_data(packet, is_string)])
	

func connect_to_url(host, protocols):
	return _ws.connect_to_url(host, protocols, false)


func disconnect_from_host():
	_ws.disconnect_from_host(1000, '')


func send_data(data, dest):
	_ws.get_peer(1).set_write_mode(_write_mode)
	_ws.get_peer(1).put_packet(Utils.encode_data(data, _write_mode))


func set_write_mode(mode):
	_write_mode = mode