extends Node2D

enum {
	EMAIL_STATE,
	PASSW_STATE
}

const EMAIL_PATTERN := "(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$)"
const PASSW_PATTERN := ""


onready var current_control = $email_state
export(int, "email", "password") var state := EMAIL_STATE setget set_state
var email:String = ''
var salt:String = ''
var passw:String = ''

var email_regex

func _ready():
	self.state = EMAIL_STATE
	
	email_regex = RegEx.new()
	email_regex.compile(EMAIL_PATTERN)


func _on_button_pressed():
	var txt = current_control.get_node('inputField').text.strip_edges()
	if state == EMAIL_STATE:
		if validate_email(txt):
			email = txt
			self.state = PASSW_STATE
		else:
			print('invalid email')
			#$email_state/inputField.
			#state.get_node('inputField').
	elif state == PASSW_STATE:
		passw = txt.sha256_text()
		print("Email: %s\nPassw: %s" % [email, passw])
		email = ''
		passw = ''
		self.state = EMAIL_STATE


func validate_email(email):
	return email_regex.search(email) != null


func set_state(val):
	state = val
	if state == EMAIL_STATE:
		current_control = $email_state
		current_control.get_node("inputField").text = ''
		$email_state.visible = true
		$passw_state.visible = false
	elif state == PASSW_STATE:
		current_control = $passw_state
		current_control.get_node("label").text = email
		$email_state.visible = false
		$passw_state.visible = true
