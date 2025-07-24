
#!/usr/bin/env python3
import os
import re

# Mapping van part tags naar variabelenamen in C
PARTS = {
    "HTML_SETTINGS_HEADER": "html_settings_header",
    "HTML_SETTINGS_CUSTOM_HTML": "html_settings_custom_html",
    "HTML_SETTINGS_BODY": "html_settings_body",
    "HTML_NETWORK_ITEM": "html_network_item",
    "HTML_SETTINGS_FOOTER": "html_settings_footer"
}


def escape_c_string(s):
    return s.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n"\n"')


def extract_parts(html):
    # Verwijder Jinja2 {% ... %} tags
    html = re.sub(r'{%.*?%}', '', html, flags=re.DOTALL)
    # Vervang Jinja2 {{ ... }} met %s
    html = re.sub(r'{{.*?}}', '%s', html)

    result = {}
    current_part = None
    buffer = []

    for line in html.splitlines():
        match = re.match(r'<!--\s*part\s+(\w+)\s*-->', line.strip())
        if match:
            if current_part and current_part in PARTS:
                result[PARTS[current_part]] = '\n'.join(buffer).strip()
            current_part = match.group(1)
            buffer = []
        else:
            buffer.append(line)

    if current_part and current_part in PARTS:
        result[PARTS[current_part]] = '\n'.join(buffer).strip()

    return result


def generate_c_output(parts):
    output = ["// Auto-generated from index.html\n"]

    for part_key in PARTS:
        name = PARTS[part_key]
        content = parts.get(name, "")

        lines = content.splitlines()
        processed_lines = [
            '"' + line.lstrip().replace('"', '\\"') + '"' for line in lines if line.strip()]
        output.append(f'const char *{name} = \n' +
                      '\n'.join(processed_lines) + ';\n')

    return '\n'.join(output)


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    content_dir = os.path.abspath(os.path.join(script_dir, '..', 'content'))
    input_file = os.path.join(content_dir, 'index.html')
    output_file = os.path.join(content_dir, 'index.html.h')

    if not os.path.exists(input_file):
        print(f"❌ File not found: {input_file}")
        return

    if os.path.exists(output_file):
        confirm = input(
            f"⚠️ '{output_file}' already exists. Overwrite? (y/n): ").strip().lower()
        if confirm != 'y':
            print("❌ Aborted.")
            return

    with open(input_file, 'r', encoding='utf-8') as f:
        html = f.read()

    parts = extract_parts(html)
    c_output = generate_c_output(parts)

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(c_output)

    print(f"✅ Generated: {output_file}")


if __name__ == '__main__':
    main()
