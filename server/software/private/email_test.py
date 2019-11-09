# import necessary packages
 
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import smtplib
import sys

if len(sys.argv) < 3:
    print("Passe a Senha e email destino como parâmetro")

# create message object instance
msg = MIMEMultipart()
 
 
message = ""
with open('cdw.html', 'r') as file:
    message = file.read()
 
# setup the parameters of the message
password = sys.argv[1]
msg['From'] = "projectdlvr@gmail.com"
msg['To'] = sys.argv[2]
msg['Subject'] = "Count Down"
 
# add in the message body
msg.attach(MIMEText(message, 'html'))
 
#create server
server = smtplib.SMTP('smtp.gmail.com: 587')
 
server.starttls()
 
# Login Credentials for sending the mail
server.login(msg['From'], password)
 
 
# send the message via the server.
server.sendmail(msg['From'], msg['To'], msg.as_string())
 
server.quit()
 
print("successfully sent email to " + str(msg['To']))