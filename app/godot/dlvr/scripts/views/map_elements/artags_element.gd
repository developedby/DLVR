extends ARTags

onready var parent := get_parent()

func _ready():
	parent.connect("state_changed", self, "_on_state_changed")

func _on_state_changed(val):
	if val == parent.STATE.ARRIVED:
		self.visible = true
		self.id = parent.qr_id
	else:
		self.visible = false