import matplotlib.pyplot as plt
import re

def advanced_tokenize(text):
    url_pattern = r'https?://[^\s]+'
    email_pattern = r'\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}\b'
    date_pattern = r'\b(?:\d{1,2}[-/]\d{1,2}[-/]\d{2,4}|\d{4}[-/]\d{1,2}[-/]\d{1,2}|(?:\d{1,2}\s+[A-Za-z]{3,9}\s+\d{4}))\b'
    word_pattern = r'\b[\w]+\b'

    combined_pattern = f"({url_pattern}|{email_pattern}|{date_pattern}|{word_pattern})"
    
    tokens = re.findall(combined_pattern, text)
    tokens = [tok[0] if isinstance(tok, tuple) else tok for tok in tokens]
    return tokens

with open("raw_dataset.txt", "r", encoding="utf-8") as f:
    raw_text = f.read()

tokens = advanced_tokenize(raw_text)

with open("tokenized.txt", "w", encoding="utf-8") as f:
    f.write(" ".join(tokens))

print(f"Tokenized {len(tokens)} words. Saved to tokenized.txt")
