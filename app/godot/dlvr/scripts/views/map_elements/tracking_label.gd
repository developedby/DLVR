extends Label

onready var parent = get_parent()

func _ready():
	parent.connect("state_changed", self, "_on_state_changed")

func _on_state_changed(val):
	visible = (val in [parent.STATE.TRACKING, parent.STATE.TRACKING_ORIGIN])