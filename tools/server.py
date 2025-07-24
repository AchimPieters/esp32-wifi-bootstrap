#!/usr/bin/env python

from collections import namedtuple
from flask import Flask, render_template, render_template_string, request
import os
import os.path
import sys


# Wi-Fi networks for testing UI
Network = namedtuple('Network', ['ssid', 'secure'])

WIFI_NETWORKS = [
    Network(ssid='SSID_Network_1', secure=False),
    Network(ssid='SSID_Network_2', secure=False),
    Network(ssid='SSID_Network_3', secure=False),
    Network(ssid='SSID_Network_4', secure=True),
    Network(ssid='SSID_Network_5', secure=True),
    Network(ssid='SSID_Network_6', secure=True),
    Network(ssid='MyWiFiNetwork_Name_With_32_Charact', secure=False),
    Network(ssid='MyWiFiNetwork_Name_With_32_Charact', secure=True),
]

# Base path to content folder
base_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..')

# Flask app configuration
app = Flask(
    __name__,
    static_folder=os.path.join(base_path, 'content'),
    template_folder=os.path.join(base_path, 'content'),
)


# Optional custom HTML block
def get_custom_html():
    if 'WIFI_CONFIG_CUSTOM_HTML' not in os.environ:
        return ''
    with open(os.environ['WIFI_CONFIG_CUSTOM_HTML']) as f:
        return f.read()


# Ternary filter for templates
@app.template_filter('ternary')
def ternary(x, true, false):
    return true if x else false


# Render settings page
def _render_settings(networks):
    if 'WIFI_CONFIG_INDEX_HTML' in os.environ:
        with open(os.environ['WIFI_CONFIG_INDEX_HTML']) as f:
            template = f.read()
        return render_template_string(
            template,
            networks=networks,
            custom_html=get_custom_html(),
        )
    return render_template(
        'index.html',
        networks=networks,
        custom_html=get_custom_html(),
    )


# Routes
@app.route('/settings', methods=['GET'])
def get_settings():
    return _render_settings(WIFI_NETWORKS)

@app.route('/settings0', methods=['GET'])
def get_settings0():
    return _render_settings([])

@app.route('/settings', methods=['POST'])
def update_settings():
    if request.form.get('password'):
        return 'Connecting to "%s", password = "%s"' % (
            request.form['ssid'],
            request.form['password']
        )
    return 'Connecting to "%s", no password' % request.form['ssid']


# Server entry point
if __name__ == '__main__':
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 5000
    app.run(host='0.0.0.0', debug=True, port=port)
