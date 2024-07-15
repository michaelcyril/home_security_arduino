from flask import Flask, request

app = Flask(__name__)

@app.route('/', methods=['POST'])
def receive_data():
    message = request.form.get('message')
    if message:
        print(f"Received message: {message}")
        return "Message received", 200
    else:
        return "No message received", 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
