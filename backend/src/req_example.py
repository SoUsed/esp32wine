import http.client

conn = http.client.HTTPSConnection('eosgl4yk85pf6a0.m.pipedream.net')
conn.request("POST", "/", '{"data": "01A3C3A3C3A3C3"}', {'Content-Type': 'application/json'})