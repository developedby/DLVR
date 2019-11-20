extends Control

export var id:int = -1 setget set_id
onready var parent = get_parent()

func _ready():
	parent.connect("state_changed", self, "_on_state_changed")
	_on_state_changed(parent.current_state)

func _on_state_changed(state):
	if state == parent.STATE.DELIVERY_REQUESTED:
		visible = true
	else:
		visible = false

func _draw():
	$label.text = ("%d " + tr("DLVR_REQ_MEM_KS")) % id

func set_id(val):
	id = val
	update()

func _on_yes_pressed():
	parent.current_state = parent.STATE.SETTING_CURRENT_LOCATION

func _on_no_pressed():
	parent.current_state = parent.STATE.IDLE
