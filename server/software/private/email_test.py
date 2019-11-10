# import necessary packages

from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import smtplib
import sys

if len(sys.argv) < 3:
    print("Passe a Senha e email destino como parâmetro")

# create message object instance
msg = MIMEMultipart("alternative")

# setup the parameters of the message
password = sys.argv[1]
msg['From'] = "projectdlvr@gmail.com"
msg['To'] = "matheusgiovannidias@live.com"
msg['Subject'] = "Subscription"

text = "Thank you"
html = """\
<!doctype html>
<html>
    <head></head>
    <body>
        <b><i>Thank you</i></b>
    </body>
</html>
"""

part1 = MIMEText(text, 'plain')
part2 = MIMEText(html, 'html')

# add in the message body
msg.attach(part1)
msg.attach(part2)

#create server
server = smtplib.SMTP('smtp.gmail.com: 587')

server.starttls()

# Login Credentials for sending the mail
server.login(msg['From'], password)


# send the message via the server.
server.sendmail(msg['From'], msg['To'], msg.as_string())

server.quit()

print("successfully sent email to " + str(msg['To']))
