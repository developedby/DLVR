extends Node

class_name Client

signal packet_received_or_timeout(status, data)
signal connected_or_timeout(status)

enum {
	OK = 0,
	TIMEOUT = 1
}

var _ws = WebSocketClient.new()
var _write_mode = WebSocketPeer.WRITE_MODE_TEXT
var last_connected_client = 0

export(float) var connection_timeout:float = 1.0 setget set_connection_timeout
export(float) var received_timeout:float = 1.0 setget set_received_timeout

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


func _ready():
	$connection_timer.connect("timeout", self, "_connection_timeout")
	$receive_timer.connect("timeout", self, "_received_timeout")
	set_connection_timeout(connection_timeout)
	set_received_timeout(received_timeout)


func _client_close_request(code, reason):
	Utils.print_log("Close code: %d, reason: %s" % [code, reason])


func _peer_connected(id):
	Utils.print_log("%s: Client just connected" % id)
	last_connected_client = id


func _exit_tree():
	_ws.disconnect_from_host(1001, "")


# warning-ignore:unused_argument
func _process(delta):
	if _ws.get_connection_status() == WebSocketClient.CONNECTION_DISCONNECTED:
		return
	_ws.poll()


func _client_connected(protocol):
	if not $connection_timer.is_stopped():
		$connection_timer.stop()
	Utils.print_log( "Client just connected with protocol: %s" % protocol)
	_ws.get_peer(1).set_write_mode(_write_mode)
	emit_signal("connected_or_timeout", OK)


func _connection_timeout():
	emit_signal("connected_or_timeout", TIMEOUT)


func _client_disconnected(clean=true):
	Utils.print_log( "Client just disconnected. Was clean: %s" % clean)
	

func _client_received(p_id = 1):
	if not $receive_timer.is_stopped():
		$receive_timer.stop()
	var packet = _ws.get_peer(p_id).get_packet()
	var is_string = _ws.get_peer(p_id).was_string_packet()
	var data = Utils.decode_data(packet, is_string)
	emit_signal("packet_received_or_timeout", OK, data)
	Utils.print_log( "Received data. BINARY: %s: %s" % [not is_string, data])


func _received_timeout():
	emit_signal("packet_received_or_timeout", TIMEOUT, '')


func set_connection_timeout(val):
	connection_timeout = val
	if $connection_timer:
		$connection_timer.wait_time = val


func set_received_timeout(val):
	received_timeout = val
	if $receive_timer:
		$receive_timer.wait_time = val


func connect_to_url(host, protocols=PoolStringArray()):
	$connection_timer.start(connection_timeout)
	return _ws.connect_to_url(host, protocols, false)


func disconnect_from_host(code:int=1000, reason:String=''):
	_ws.disconnect_from_host(code, reason)


func send_data(data, peer=1, write_mode=WebSocketPeer.WRITE_MODE_TEXT):
	_write_mode = write_mode
	if _ws.get_peer(peer).is_connected_to_host():
		_ws.get_peer(peer).set_write_mode(_write_mode)
		_ws.get_peer(peer).put_packet(Utils.encode_data(data, _write_mode))


func set_write_mode(mode):
	_write_mode = mode

func start_receive_timer():
	$receive_timer.start(received_timeout)
