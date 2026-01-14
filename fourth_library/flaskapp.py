from flask import Flask, jsonify, request
from flask_sqlalchemy import SQLAlchemy
import os
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__)

app.config["SQLALCHEMY_DATABASE_URI"] = os.getenv("DB_URL", "sqlite:///library.db")
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False

db = SQLAlchemy(app)


class Book(db.Model):
    __tablename__ = "books"

    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(200), nullable=False)
    author = db.Column(db.String(200), nullable=False)
    isbn = db.Column(db.String(50), unique=True, nullable=False)
    category = db.Column(db.String(100))
    total_copies = db.Column(db.Integer, nullable=False)
    available_copies = db.Column(db.Integer, nullable=False)

with app.app_context():
    db.create_all()


@app.get("/health")
def health():
    return jsonify({"status": "ok"})


@app.get("/books")
def list_books():
    books = Book.query.all()
    result = []

    for b in books:
        result.append({
            "id": b.id,
            "title": b.title,
            "author": b.author,
            "isbn": b.isbn,
            "category": b.category,
            "total_copies": b.total_copies,
            "available_copies": b.available_copies
        })

    return jsonify(result), 200


@app.get("/books/add")
def add_book_sample():
    count = Book.query.count()
    isbn = f"TEST-{count+1:03d}"

    book = Book(
        title=f"Sample Book {count+1}",
        author="Tester",
        isbn=isbn,
        category="Demo",
        total_copies=3,
        available_copies=3
    )

    db.session.add(book)
    db.session.commit()

    return jsonify({
        "message": "book added",
        "book_id": book.id,
        "isbn": book.isbn
    }), 201

@app.post("/books")
def create_book():
    data = request.get_json() or {}

    required_fields = ["title", "author", "isbn", "total_copies"]
    for field in required_fields:
        if field not in data:
            return jsonify({"error": f"{field} is required"}), 400

    if Book.query.filter_by(isbn=data["isbn"]).first():
        return jsonify({"error": "ISBN already exists"}), 400

    try:
        total = int(data["total_copies"])
    except Exception:
        return jsonify({"error": "total_copies must be a number"}), 400

    if total <= 0:
        return jsonify({"error": "total_copies must be >= 1"}), 400

    book = Book(
        title=data["title"],
        author=data["author"],
        isbn=data["isbn"],
        category=data.get("category"),
        total_copies=total,
        available_copies=total
    )

    db.session.add(book)
    db.session.commit()

    return jsonify({"message": "book created", "book_id": book.id}), 201

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=8000, debug=True)
