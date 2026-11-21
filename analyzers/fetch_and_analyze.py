import urllib.request
import sys

url = "https://docs.google.com/spreadsheets/d/1A-Ld-1_suLp5Xl2PM6i5YwdtDr-CeldI07kmXP56zAg/export?format=csv&gid=735051613"

try:
    req = urllib.request.Request(
        url,
        headers={'User-Agent': 'Mozilla/5.0'}
    )
    with urllib.request.urlopen(req) as response:
        content = response.read().decode('utf-8', errors='ignore')
        with open("thrust_bench_data.csv", "w", encoding="utf-8") as f:
            f.write(content)
        print("Successfully downloaded. Length:", len(content))
        print("First 1000 characters:")
        print(content[:1000])
except Exception as e:
    print("Error fetching spreadsheet:", e)
