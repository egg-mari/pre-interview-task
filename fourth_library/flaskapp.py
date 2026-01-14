from flask import Flask, jsonify
from flask_sqlalchemy import SQLAlchemy
import os
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__)

app.config["SQLALCHEMY_DATABASE_URI"] = os.getenv("DB_URL", "sqlite:///library.db")
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False

db = SQLAlchemy(app)

@app.get("/health")
def health():
    return jsonify({"status": "ok"})

@app.get("/books")
def list_books():
    return jsonify([])

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=8000, debug=True)
