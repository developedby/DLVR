extends LineEdit

class_name EmailField

const EMAIL_PATTERN := "(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$)"

const EMPTY_COLOR := Color(0.804, 0.804, 0.804)
const VALID_COLOR := Color(0.518, 0.714, 0.898)
const INVALID_COLOR := Color(0.898, 0.714, 0.518)

var _re:RegEx = null

var valid := false


func _ready():
	_re = RegEx.new()
	# warning-ignore: return_value_discarded
	_re.compile(EMAIL_PATTERN)


func set_valid():
	get_stylebox("focus").border_color = VALID_COLOR
	valid = true


func set_invalid():
	get_stylebox("focus").border_color = INVALID_COLOR
	valid = false


func set_empty():
	get_stylebox("focus").border_color = EMPTY_COLOR
	valid = false


func _draw():
	var value = self.text.strip_edges()
	if value == "":
		set_empty()
	elif _re.search(value):
		set_valid()
	else:
		set_invalid()
