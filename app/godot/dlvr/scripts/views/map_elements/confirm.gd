extends Control

onready var parent = get_parent()
var email:String = ""

func _ready():
	parent.connect("state_changed", self, "_on_state_changed")
	_on_state_changed(parent.current_state)

func _on_state_changed(state):
	if state == parent.STATE.DELIVERY_REQUESTED:
		visible = true
	else:
		visible = false
		email = ""

func _draw():
	$label.text = "%s\n%s" % [email, tr("DLVR_REQ_MEM_KS")]

func _on_yes_pressed():
	parent.current_state = parent.STATE.SETTING_CURRENT_LOCATION

func _on_no_pressed():
	parent.current_state = parent.STATE.IDLE
	var data = {"path": "/delivery/response", "cookie": DLVR.cookie, "id": parent.last_request_id, "accept": false}
	DLVR.client.send_data(JSON.print(data))
