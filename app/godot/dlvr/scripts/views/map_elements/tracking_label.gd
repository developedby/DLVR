extends Label

onready var parent = get_parent()

func _ready():
	parent.connect("state_changed", self, "_on_state_changed")

func _on_state_changed(val):
	if val == parent.STATE.IDLE:
		self.text = "IDLE_KS"
	elif val == parent.STATE.ORIGIN_SET:
		self.text = "ORIGINSET_KS"
	elif val == parent.STATE.WAITING_RECEIVER:
		self.text = "WAITINGRECEIVER_KS"
	elif val in [parent.STATE.TRACKING, parent.STATE.TRACKING_ORIGIN]:
		self.text = "TRACKING_KS"
	elif val == parent.STATE.WAITING_SEND:
		self.text = "WAITINGSEND_KS"
	elif val == parent.STATE.OPENED:
		self.text = "OPENED_KS"
	else:
		self.text = ""
		