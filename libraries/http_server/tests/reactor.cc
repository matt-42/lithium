

enum status {
  READ,
  WRITE,
  EOF
};

event_loop([] (int fd, status s, auto event_queue) {

  event_queue.add(new_fd, RW);
  event_queue.mod(new_fd, RW);
  event_queue.del(new_fd, RW);
});
