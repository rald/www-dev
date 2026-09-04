#!/usr/bin/env python3
import cgi
import json
import os
import sys

# Ensure standard output handles UTF-8 cleanly
sys.stdout = open(sys.stdout.fileno(), mode='w', encoding='utf-8', buffering=1)

DATA_FILE = os.path.join(os.path.dirname(__file__), '../data/sites.json')

def load_db():
    if not os.path.exists(DATA_FILE):
        return []
    try:
        with open(DATA_FILE, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception:
        return []

def save_db(data):
    os.makedirs(os.path.dirname(DATA_FILE), exist_ok=True)
    with open(DATA_FILE, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2)

def main():
    form = cgi.FieldStorage()
    action = form.getvalue('action', 'search')
    
    response = {"status": "success", "data": []}

    if action == 'submit':
        url = form.getvalue('url', '').strip()
        title = form.getvalue('title', '').strip()
        if url:
            if not url.startswith(('http://', 'https://')):
                url = 'http://' + url
            title = title if title else url
            
            db = load_db()
            if not any(item['url'] == url for item in db):
                db.append({"title": title, "url": url})
                save_db(db)
                response["message"] = "Site indexed successfully."
            else:
                response["status"] = "error"
                response["message"] = "URL already exists in index."
        else:
            response["status"] = "error"
            response["message"] = "URL cannot be empty."

    elif action == 'search':
        query = form.getvalue('q', '').lower().strip()
        db = load_db()
        if query:
            response["data"] = [
                item for item in db 
                if query in item['title'].lower() or query in item['url'].lower()
            ]
        else:
            response["data"] = db

    print("Content-Type: application/json; charset=utf-8\n")
    print(json.dumps(response))

if __name__ == '__main__':
    main()
