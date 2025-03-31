import torch
from transformers import AutoTokenizer, AutoModelForCausalLM
from flask import Flask, render_template, request

#app = Flask(__name__)

tokenizer = AutoTokenizer.from_pretrained("deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B")
model = AutoModelForCausalLM.from_pretrained("deepseek-ai/DeepSeek-R1-Distill-Qwen-1.5B")
from transformers import pipeline
pipe=pipeline("text-generation",model=model,tokenizer=tokenizer,truncation=True)
def generate_response(prompt):
    return pipe(prompt,max_length=1000,num_return_sequences=1)[0]["generated_text"]
print(generate_response("hello"))
#@app.route("/", methods=["GET", "POST"])
#def chat():
#    response = ""
#    if request.method == "POST":
#        user_input = request.form["user_input"]
#        response = generate_response(user_input)
#    return render_template("chat.html", response=response)

#if __name__ == "__main__":
#    app.run(debug=True)




