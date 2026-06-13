import os
from flask import Flask, send_file, abort

app = Flask(__name__)

# Search across common output targets to find the exact binary path
POSSIBLE_PATHS = [
    "build/CymuxV2.apk",
    "app/build/outputs/apk/debug/app-debug.apk",
    "CymuxV2.apk"
]

@app.route('/', defaults={'path': ''})
@app.route('/<path:path>')
def force_download(path):
    for apk_path in POSSIBLE_PATHS:
        if os.path.exists(apk_path):
            return send_file(
                apk_path,
                mimetype="application/vnd.android.package-archive",
                as_attachment=True,
                download_name="CymuxV2.apk"
            )
    abort(404, description="Target APK artifact not found on disk.")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, threaded=True)
