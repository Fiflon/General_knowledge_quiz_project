import socket
import threading
import tkinter as tk
import re
from tkinter import scrolledtext, messagebox, simpledialog, ttk
import struct


def send_string(sock, message):
    size_of_msg = len(message)
    print(type(size_of_msg))
    print(f"Wysyłanie wiadomości o rozmiarze: {size_of_msg}")
    sock.sendall(struct.pack('i', size_of_msg))
    sock.sendall(message.encode('utf-8'))
    print(f"Wiadomość wysłana: {message}")


def recv_string(sock):
    print("Czekam na rozmiar wiadomości...")
    size_of_msg_data = struct.unpack('i', sock.recv(4))[0]
    if not size_of_msg_data:
        raise ConnectionError("Didin't receive data in the right format.")
    
    size_of_msg = size_of_msg_data
    print(f"Otrzymano rozmiar wiadomości: {size_of_msg}")
    message_data = sock.recv(size_of_msg)
    print(f"Otrzymane dane wiadomości: {message_data}")

    if len(message_data) != size_of_msg:
        raise ValueError("Didn't received the whole message.")
    # przesyl ponowny?

    return message_data


class NetcatClientApp:
    def __init__(self, root):
        self.root = root
        self.root.title("General Knowledge Quiz Project")

        tk.Label(root, text="Host:").grid(row=0, column=0)
        self.host_entry = tk.Entry(root)
        self.host_entry.grid(row=0, column=1)

        tk.Label(root, text="Port:").grid(row=1, column=0)
        self.port_entry = tk.Entry(root)
        self.port_entry.grid(row=1, column=1)

        self.text_area = scrolledtext.ScrolledText(root, wrap=tk.WORD, state='disabled', height=10)
        self.text_area.grid(row=2, column=0, columnspan=2, pady=10)

        self.message_entry = tk.Entry(root)
        self.message_entry.grid(row=3, column=0, padx=5, pady=5)

        self.send_button = tk.Button(root, text="Send", command=self.send_message, state='disabled')
        self.send_button.grid(row=3, column=1, padx=5, pady=5)

        self.start_button = tk.Button(root, text="Connect", command=self.connect_to_server)
        self.start_button.grid(row=4, column=0, columnspan=2, pady=10)

        # Questions
        self.question_frame = tk.Frame(root)
        self.question_frame.grid(row=5, column=0, columnspan=2, pady=10)

        tk.Label(self.question_frame, text="Question: ").grid(row=0, column=0, sticky='w')
        self.question_number_label = tk.Label(self.question_frame, text="")
        self.question_number_label.grid(row=0, column=1, sticky='w')
        self.current_question_number = None 

        tk.Label(self.question_frame, text="Difficulty:").grid(row=0, column=2, sticky='w', padx=10)
        self.difficulty_label = tk.Label(self.question_frame, text="")
        self.difficulty_label.grid(row=0, column=3, sticky='w')

        self.question_label = tk.Label(self.question_frame, text="", wraplength=400, justify='left', anchor='w')
        self.question_label.grid(row=1, column=0, columnspan=4, sticky='w')
                

        self.answer_buttons = []
        for i, label in enumerate(["A", "B", "C", "D"]):
            btn = tk.Button(self.question_frame, text=label, command=lambda idx=i: self.answer_selected(idx))
            btn.grid(row=2 + i, column=0, columnspan=4, sticky='ew', pady=2)
            btn.config(state='disabled')
            self.answer_buttons.append(btn)

        # Ranking
        self.rank_frame = tk.Frame(root)
        self.rank_frame.grid(row=6, column=0, columnspan=2, pady=10)

        tk.Label(self.rank_frame, text="Ranking:").grid(row=0, column=0, sticky='w')

        self.rank_tree = ttk.Treeview(self.rank_frame, columns=("Nickname", "Points"), show="headings")
        self.rank_tree.grid(row=1, column=0, columnspan=2, pady=5)

        self.rank_tree.heading("Nickname", text="Nickname")
        self.rank_tree.heading("Points", text="Points")

        self.rank_tree.column("Nickname", width=150)
        self.rank_tree.column("Points", width=80)

        
        self.username = None
        self.client_socket = None
        self.receive_thread = None
    
    
    def is_valid_nickname(self, nickname):
        if not nickname:
            return False
        if len(nickname) > 20 or len(nickname) < 4:
            return False
        if not re.match(r'^[a-zA-Z][a-zA-Z0-9_]+$', nickname):
            return False
        return True


    def set_username(self):
        while True:
            new_username = tk.simpledialog.askstring("Nickname", "Set your nickname:")
            if self.is_valid_nickname(new_username):
                self.username = new_username
                send_string(self.client_socket, f"nic|{self.username}|")
                break
            else:
                messagebox.showerror("Invalid Username", "Your nickname is invalid. Requirements:\n"
                                                        "- Not empty\n"
                                                        "- Less than 20 and more than 4 characters\n"
                                                        "- Without special characters and spaces")


    def display_question(self, question_number, question_text, difficulty, answers):
        self.current_question_number = question_number 
        self.question_number_label.config(text=question_number)
        self.difficulty_label.config(text=difficulty)
        self.question_label.config(text=question_text)

        for btn in self.answer_buttons:
            btn.config(state='normal')

        for btn, answer in zip(self.answer_buttons, answers):
            btn.config(text=answer)


    def answer_selected(self, index):
        if self.client_socket and self.current_question_number:
            if index == 0:
                letter = "a"
            elif index == 1:
                letter = "b"
            elif index == 2:
                letter = "c"
            elif index == 3:
                letter = "d"
            answer_message = f"ans|{self.current_question_number}|{letter}|"
            send_string(self.client_socket, answer_message)
            self.append_text(f"You selected answer {letter} for question {self.current_question_number}")

            for btn in self.answer_buttons:
                btn.config(state='disabled')


    def update_ranking(self, message):
        parts = message.split('|')[1:] 

        if parts[-1] == "":
            parts = parts[:-1]

        for row in self.rank_tree.get_children():
            self.rank_tree.delete(row)

        for part in parts:
            nickname, points = part.split(':')
            self.rank_tree.insert("", "end", values=(nickname, points))


    def connect_to_server(self):
        host = self.host_entry.get()
        port = self.port_entry.get()

        if not host or not port:
            messagebox.showerror("Error", "Insert host and port!")
            return

        try:
            port = int(port)
            print(f"Łączenie z serwerem {host}:{port}")
            self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.client_socket.connect((host, port))
            
            self.set_username()

            self.start_button.config(state='disabled')
            self.send_button.config(state='normal')
            self.append_text(f"Connected to the server {host}:{port}\n")
            print(f"Połączono z {host}:{port} jako {self.username}")

            self.receive_thread = threading.Thread(target=self.receive_messages, daemon=True)
            self.receive_thread.start()

        except Exception as e:
            messagebox.showerror("Error", f"Failed to connect to the server: {e}")


    def parse_message(self, message):
        print(f"Parsujemy wiadomosc: {message}")
        if message.startswith("nic|"):
            status = message.split('|')[1]
            print(f"Status: {status}")
            if status == "0" or status == "4" or status == "5":
                self.append_text(f"Username set: {self.username}")
                self.root.title(f"Quiz - {self.username}")
            else:
                if status == "1":
                    messagebox.showwarning("Invalid Username", "It's already taken.")
                elif status == "2":
                    messagebox.showwarning("Invalid Username", "It has special characters.")
                elif status == "3":
                    messagebox.showwarning("Invalid Username", "It's too long or too short.")
                self.set_username()
        elif message.startswith("que|"):
            parts = message.split('|')
            question_number = parts[1]
            question_text = parts[2]
            answers = parts[3:7]
            difficulty = parts[7]
            print(f"{question_number}|{question_text}|{answers}|{difficulty}")
            self.display_question(question_number, question_text, difficulty, answers)
        elif message.startswith("ans|"):
            parts = message.split('|')
            status = parts[1]
            if status == '0':
                self.append_text("Correct answer!")
            elif status == '1':
                self.append_text("Incorrect answer!")
            elif status == '2':
                self.append_text("Invalid question number!")
            elif status == '3':
                self.append_text("The game is not currently running!")
        elif message.startswith("rank|"):
            self.update_ranking(message)
        else:
            self.append_text(message)


    def receive_messages(self):
        try:
            print("Rozpoczęto odbieranie wiadomości.")
            while True:
                message = recv_string(self.client_socket).decode()
                if not message:
                    break
                print(f"Otrzymana wiadomość: {message}")
                # self.append_text(message)
                self.parse_message(message)
        except Exception as e:
            self.append_text(f"Error while receiving a message: {e}\n")
            print(f"Błąd podczas odbierania wiadomości: {e}")
        finally:
            self.client_socket.close()
            self.append_text("Połączenie zostało zakończone.\n")
            print("Połączenie zostało zamknięte.")


    def send_message(self):
        message = self.message_entry.get()
        if message:
            try:
                send_string(self.client_socket, message)
                self.append_text(f"{self.username}: {message}\n")
                self.message_entry.delete(0, tk.END)
            except Exception as e:
                self.append_text(f"Błąd podczas wysyłania wiadomości: {e}\n")


    def append_text(self, text):
        self.text_area.config(state='normal')
        self.text_area.insert(tk.END, text + '\n')
        self.text_area.config(state='disabled')
        self.text_area.see(tk.END)


if __name__ == "__main__":
    root = tk.Tk()
    app = NetcatClientApp(root)
    root.mainloop()
