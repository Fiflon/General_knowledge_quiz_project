import socket
import threading
import tkinter as tk
import re
from tkinter import scrolledtext, messagebox, simpledialog
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


def is_valid_nickname(nickname):
    if not nickname:
        return False
    if len(nickname) > 20 or len(nickname) < 4:
        return False
    if not re.match(r'^[a-zA-Z][a-zA-Z0-9_]+$', nickname):
        return False
    return True


class NetcatClientApp:
    def __init__(self, root):
        self.root = root
        self.root.title("General Knowledge Quiz Project")

        # Interfejs użytkownika
        tk.Label(root, text="Host:").grid(row=0, column=0)
        self.host_entry = tk.Entry(root)
        self.host_entry.grid(row=0, column=1)

        tk.Label(root, text="Port:").grid(row=1, column=0)
        self.port_entry = tk.Entry(root)
        self.port_entry.grid(row=1, column=1)

        self.text_area = scrolledtext.ScrolledText(root, wrap=tk.WORD, state='disabled')
        self.text_area.grid(row=2, column=0, columnspan=2, pady=10)

        self.message_entry = tk.Entry(root)
        self.message_entry.grid(row=3, column=0, padx=5, pady=5)

        self.send_button = tk.Button(root, text="Send", command=self.send_message, state='disabled')
        self.send_button.grid(row=3, column=1, padx=5, pady=5)

        self.start_button = tk.Button(root, text="Connect", command=self.connect_to_server)
        self.start_button.grid(row=4, column=0, columnspan=2, pady=10)

        self.username = None
        self.client_socket = None
        self.receive_thread = None


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
            
            # Set the nickname with validation
            while True:
                self.username = tk.simpledialog.askstring("Nickname", "Set your nickname:")
                if is_valid_nickname(self.username):
                    break
                messagebox.showwarning("Invalid Nickname", "Nickname must be 4-20 characters long and can only contain letters, numbers, and underscores. Please try again.")

            print(f"Tuż przed wysłaniem nicku nic|{self.username}|")
            send_string(self.client_socket, f"nic|{self.username}|")
            print(f"Nick wyslany")

            self.start_button.config(state='disabled')
            self.send_button.config(state='normal')
            self.append_text(f"Connected to the server {host}:{port}\n")
            print(f"Połączono z {host}:{port} jako {self.username}")

            self.receive_thread = threading.Thread(target=self.receive_messages, daemon=True)
            self.receive_thread.start()

        except Exception as e:
            messagebox.showerror("Error", f"Failed to connect to the server: {e}")


    def receive_messages(self):
        try:
            print("Rozpoczęto odbieranie wiadomości.")
            while True:
                message = recv_string(self.client_socket).decode()
                if not message:
                    break
                print(f"Otrzymana wiadomość: {message}")
                self.append_text(message)
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