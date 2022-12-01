import requests
import random

word_site = "https://www.mit.edu/~ecprice/wordlist.10000"
word_list = []
FILENAME = "words.txt"
response = requests.get(word_site)
WORDS = response.content.splitlines()

while len(word_list) < 50:
    word = random.choice(WORDS).decode("utf-8")
    if len(word) in range(3, 31):
        word_list.append(word)
file = open(FILENAME, "w")
file.write("")

for i in range(len(word_list)):
    word = word_list[i]
    file.write(word + " " + word + ".png")
    if i != len(word_list) - 1:
        file.write("\n")
file.close()