#!/usr/bin/env python3
# Auto-increment version and update changelog for GitHub Actions release
import re
import sys
from datetime import date

changelog = 'CHANGELOG.md'

with open(changelog, 'r+') as f:
    content = f.read()
    m = re.search(r'v(\d+)\.(\d+)\.(\d+)', content)
    if not m:
        print('No version found in changelog!')
        sys.exit(1)
    major, minor, patch = map(int, m.groups())
    patch += 1
    new_version = f'v{major}.{minor}.{patch}'
    today = date.today().isoformat()
    new_entry = f"\n## {new_version} ({today})\n- Minor update, auto-version bump for release\n"
    content = re.sub(r'(# Changelog\n)', r'\1' + new_entry, content)
    f.seek(0)
    f.write(content)
    f.truncate()
print(f'Bumped version to {new_version}')
