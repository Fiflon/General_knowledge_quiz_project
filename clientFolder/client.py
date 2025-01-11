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
    try:
        print("Czekam na rozmiar wiadomości...")
        size_of_msg_data = sock.recv(4)
        if not size_of_msg_data:
            raise ConnectionError("The connection to the server was interrupted.")

        size_of_msg = struct.unpack('i', size_of_msg_data)[0]
        print(f"Otrzymano rozmiar wiadomości: {size_of_msg}")
        message_data = sock.recv(size_of_msg)
        print(f"Otrzymane dane wiadomości: {message_data}")

        if len(message_data) != size_of_msg:
            raise ValueError("Didn't receive the whole message.")

        return message_data
    except ConnectionError as e:
        raise ConnectionError(str(e))
    except struct.error:
        raise ConnectionError("The connection to the server was interrupted due to invalid data.")


class QuizClient:
    def __init__(self, root):
        self.root = root
        self.root.title("General Knowledge Quiz Project")

        tk.Label(root, text="Host:").grid(row=0, column=0, pady=5)
        self.host_entry = tk.Entry(root)
        self.host_entry.grid(row=0, column=1)
        self.host_entry.insert(0, "127.0.0.1")

        tk.Label(root, text="Port:").grid(row=1, column=0)
        self.port_entry = tk.Entry(root)
        self.port_entry.grid(row=1, column=1)
        self.port_entry.insert(0, "9867")

        self.start_button = tk.Button(root, text="Connect", command=self.connect_to_server)
        self.start_button.grid(row=2, column=1, columnspan=2, pady=5)

        self.disconnect_button = tk.Button(root, text="Disconnect", command=self.disconnect_from_server, state='disabled')
        self.disconnect_button.grid(row=2, column=0, columnspan=2, pady=5)

        self.text_area = scrolledtext.ScrolledText(root, wrap=tk.WORD, state='disabled', height=10)
        self.text_area.grid(row=3, column=0, columnspan=2, pady=10)

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

        self.time_progress = ttk.Progressbar(self.question_frame, maximum=100, length=400)
        self.time_progress.grid(row=6, column=0, columnspan=4, pady=5)
                

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
        self.timer_running = False
        self.recv_thread_flag = False
        self.previous_players = set()
    
    
    def is_valid_nickname(self, nickname):
        if not nickname:
            return False
        if len(nickname) > 20 or len(nickname) < 4:
            return False
        if not re.match(r'^[a-zA-Z][a-zA-Z0-9_]+$', nickname):
            return False
        return True

       
    def set_username(self):
        dialog = tk.Toplevel(self.root)
        dialog.title("Set Nickname")
        tk.Label(dialog, text="Enter your nickname:").pack(pady=10)
        entry = tk.Entry(dialog)
        entry.pack(pady=5)

        def confirm():
            new_username = entry.get()
            if self.is_valid_nickname(new_username):
                self.username = new_username
                send_string(self.client_socket, f"nic|{self.username}|")
                dialog.destroy()
            else:
                messagebox.showerror("Invalid Username", "Your nickname is invalid. Requirements:\n"
                                                        "- Not empty\n"
                                                        "- Less than 20 and more than 4 characters\n"
                                                        "- Without special characters and spaces")

        tk.Button(dialog, text="Confirm", command=confirm).pack(pady=10)
        dialog.wait_window()


    def reset_timer(self):
        self.timer_running = False
        self.time_progress["value"] = 0
        self.root.update_idletasks()


    def start_timer(self, duration):
        self.reset_timer()
        self.timer_running = True
        step = 100 / (duration * 10)

        def update_progress(current_time):
            if current_time < duration:
                self.time_progress["value"] += step
                self.root.update_idletasks()
                self.root.after(100, update_progress, current_time + 0.1)

        update_progress(0)


    def display_question(self, question_number, question_text, difficulty, answers):
        self.current_question_number = question_number 
        self.question_number_label.config(text=question_number)
        self.difficulty_label.config(text=f"{difficulty}/3")
        self.question_label.config(text=question_text)

        for btn in self.answer_buttons:
            btn.config(state='normal')

        for btn, answer in zip(self.answer_buttons, answers):
            btn.config(text=answer)
        
        self.start_timer(duration=15)


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
        
        current_players = set()

        for row in self.rank_tree.get_children():
            self.rank_tree.delete(row)

        for part in parts:
            nickname, points = part.split(':')
            self.rank_tree.insert("", "end", values=(nickname, points))
            current_players.add(nickname)

        new_players = current_players - self.previous_players
        self.previous_players = current_players

        for new_player in new_players:
            if new_player != self.username:
                self.append_text(f"Player {new_player} is in the game! He started playing from the question {self.current_question_number or 1}.")

    
    def get_winner(self):
        max_points = -1
        winner = ""
        for row in self.rank_tree.get_children():
            nickname, points = self.rank_tree.item(row, "values")
            points = int(points)
            if points > max_points:
                max_points = points
                winner = nickname
            elif points == max_points:
                winner += "|" + nickname
        return winner, max_points


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
            self.host_entry.config(state='disabled')
            self.port_entry.config(state='disabled')
            self.disconnect_button.config(state='normal')
            self.append_text(f"Connected to the server {host}:{port}\n")
            print(f"Połączono z {host}:{port} jako {self.username}")

            self.recv_thread_flag = True
            self.receive_thread = threading.Thread(target=self.receive_messages, daemon=True)
            self.receive_thread.start()
        
        except ConnectionRefusedError:
            error_message = "Failed to connect to the server. The server is offline or unreachable."
            self.append_text(f"{error_message}\n")
            messagebox.showerror("Connection Error", error_message)
            print(error_message)

        except Exception as e:
            messagebox.showerror("Error", f"Failed to connect to the server: {e}")

    
    def disconnect_from_server(self):
        if self.client_socket:
            try:
                send_string(self.client_socket, f"exi|{self.username}|")
                self.recv_thread_flag = False
                self.client_socket.close()
                print("Połączenie zostało zamknięte.")
            except Exception as e:
                self.append_text(f"Błąd podczas rozłączania: {e}\n")
                print(f"Błąd rozłączania: {e}")
            finally:
                self.client_socket = None
                self.start_button.config(state='normal')
                self.disconnect_button.config(state='disabled')
                self.host_entry.config(state='normal')
                self.port_entry.config(state='normal')
                for btn in self.answer_buttons:
                    btn.config(state='disabled')
                self.reset_timer()


    def parse_message(self, message):
        print(f"Parsujemy wiadomosc: {message}")
        if message.startswith("nic|"):
            status = message.split('|')[1]
            print(f"Status: {status}")
            if status == "0" or status == "4" or status == "5":
                self.append_text(f"Welcome to the General Knowledge Quiz!")
                self.append_text(f"Username set: {self.username}")
                self.append_text(f"There must be at least 3 players to start the game.")
                self.root.title(f"Quiz - {self.username}")
                if status == "4":
                    self.append_text(f"The game has already started. No worries, you will join from the next question.")
                if status == "5":
                    self.append_text(f"The 20s countdown to start the game has already started. Prepare for the start of the game.")
            else:
                if status == "1":
                    messagebox.showwarning("Invalid Username", "It's already taken.")
                elif status == "2":
                    messagebox.showwarning("Invalid Username", "It has special characters.")
                elif status == "3":
                    messagebox.showwarning("Invalid Username", "It's too long or too short.")
                self.set_username()
        elif message.startswith("gam|"):
            status = message.split('|')[1]
            if status == "0":
                self.append_text(f"The game started.")
                self.reset_timer()
                self.rank_frame.children["!label"].config(text="Ranking")
            elif status == "1":
                self.append_text(f"The countdown paused due to too few players (3 players are needed to start a game).")
                self.reset_timer()
            elif status == "2":
                self.append_text(f"The 20s countdown started now. Prepare for the start of the game.")
                self.start_timer(duration=20)
            elif status == "3" or "4":
                for btn in self.answer_buttons:
                    btn.config(state='disabled')
                self.rank_frame.children["!label"].config(text="Final ranking")
                if status == "3":
                    winner, max_points = self.get_winner()
                    if winner != "":
                        if "|" in winner:
                            self.append_text(f"Game over! The winners are {', '.join(winner.split('|'))} with {max_points} points!")
                        else:
                            self.append_text(f"Game over! The winner is {winner} with {max_points} points!")
                    else:
                        self.append_text("Game over! No winner could be determined.")
                else:
                    self.append_text(f"Game over! The game ended due to too few players (there have to be at least 2 players).")
        elif message.startswith("dis|") or message.startswith("exi|"):
            disconnected_player = message.split('|')[1]
            self.append_text(f"Player {disconnected_player} disconnected.")
        elif message.startswith("que|"):
            parts = message.split('|')
            question_number = parts[1]
            question_text = parts[2]
            answers = parts[3:7]
            difficulty = parts[7]
            print(f"{question_number}|{question_text}|{answers}|{difficulty}")
            self.display_question(question_number, question_text, difficulty, answers)
        elif message.startswith("ans|"):
            status = message.split('|')[1]
            if status == '0':
                self.append_text("Correct answer!")
            elif status == '1':
                self.append_text("Incorrect answer!")
            elif status == '2':
                self.append_text("Invalid question number.")
            elif status == '3':
                self.append_text("The game is not currently running.")
        elif message.startswith("rank|"):
            self.update_ranking(message)
        else:
            self.append_text(message)


    def receive_messages(self):
        try:
            print("Rozpoczęto odbieranie wiadomości.")
            while self.recv_thread_flag:
                message = recv_string(self.client_socket).decode()
                if not message:
                    break
                print(f"Otrzymana wiadomość: {message}")
                self.parse_message(message)
        except ConnectionError as e:
            if self.recv_thread_flag:
                self.append_text(f"Connection error: {e}")
                print(f"Błąd połączenia: {e}")
        except Exception as e:
            self.append_text(f"Unexpected error while receiving a message: {e}\n")
            print(f"Niespodziewany błąd: {e}")
        finally:
            if self.recv_thread_flag:
                self.client_socket.close()
            self.client_socket = None
            self.append_text("You disconnected from the server.\n")
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

    def on_closing(self):
        self.disconnect_from_server()
        self.root.destroy()

if __name__ == "__main__":



    root = tk.Tk()
    app = QuizClient(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()
