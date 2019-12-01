extends Control

onready var parent = get_parent()

func _ready():
	parent.connect("state_changed", self, "_on_state_changed")
	_on_state_changed(parent.current_state)

func _on_state_changed(state):
	if state == parent.STATE.RECEIVER_REFUSED:
		visible = true
	else:
		visible = false

func _on_ok_pressed():
	self.parent.set_state(parent.STATE.IDLE)
