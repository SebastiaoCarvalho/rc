import requests
import random
import shutil
import os

ACCESSKEY = "7bybuvKRX51UR7ZNefbHL48A1eSwo5nWG8HKtV-mDhI"

def getImages(word):
    url = "https://api.unsplash.com/search/photos?query=" + word + "&client_id=" + ACCESSKEY
    response = requests.get(url)
    data = response.json()
    return data

img_dir = "./images/"

if not os.path.exists(img_dir):
    os.makedirs(img_dir)

word_site = "https://www.mit.edu/~ecprice/wordlist.10000"
word_list = []
FILENAME = "words.txt"
response = requests.get(word_site)
WORDS = response.content.splitlines()

while len(word_list) < 25:
    word = random.choice(WORDS).decode("utf-8")
    if len(word) in range(3, 31):
        word_list.append(word)
file = open(FILENAME, "w")
file.write("")

for i in range(len(word_list)):
    word = word_list[i]
    results = getImages(word)["results"]
    if len(results) == 0:
        print(word)
        continue
    link = results[0]["urls"]["full"]
    response = requests.get(link, stream=True)
    with open(img_dir + word + ".jpg", "wb") as out_file:
        shutil.copyfileobj(response.raw, out_file)
    file.write(word + " " + word + ".jpg")
    if i != len(word_list) - 1:
        file.write("\n")
file.close()