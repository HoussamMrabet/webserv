import cgi
import cgitb

cgitb.enable()

name = "test"

form = cgi.FieldStorage()
name = form.getvalue("name")

print("<html>")
print("<body>")
print("<h1>Hello, {}!</h1>".format(name))
print("</body>")
print("</html>")