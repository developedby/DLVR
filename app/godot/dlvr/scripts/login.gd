extends Control

onready var sign_in_button := $vbox/sign_in
onready var email_field := $vbox/email_margin/email
onready var password_field := $vbox/password_margin/password


func _ready():
	sign_in_button.connect("pressed", self, "on_sign_in_pressed")


func on_sign_in_pressed():
	var email:String = email_field.text.strip_edges()
	var passw:String = password_field.text
	print("%s\n%s" % [email, passw.sha256_text()])


