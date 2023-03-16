import os
import os.path
import time


class Pidfile:

    def __init__(self, filename: str, retry_timeout_seconds: int = 5):
        self.filename = filename
        self.retry_timeout_seconds = retry_timeout_seconds

    def __del__(self):
        self.remove()


    def _read(self):
        if self.check_exists():
            with open(self.filename, 'r') as f:
                pid = f.read()
                return pid

        return None


    def _write(self, pid: int, overwrite: bool = False):
        if self.check_exists() and not overwrite:
            raise Exception(f'PID file {self.filename} already exists')

        with open(self.filename, 'w') as f:
            f.write(f'{pid}')


    def check_exists(self):
        return os.path.exists(self.filename)


    def create(self, overwrite: bool = False):
        self._write(pid=os.getpid(), overwrite=overwrite)


    def remove(self):
        if self.check_exists():
            os.remove(self.filename)


    def check_and_wait(self, number_of_retries: int = 5):
        retry_count = 0

        while self.check_exists():
            retry_count += 1

            stale_pid = self._read()

            if not os.path.exists(os.path.join('/proc', stale_pid)):
                print(f'stale PID file {self.filename} found, removing and continuing')
                self.remove()
                break

            if retry_count > number_of_retries:
                time_waited = retry_count * self.retry_timeout_seconds
                raise Exception(f'PID file {self.filename} existed for the past {time_waited} seconds - giving up.')

            time.sleep(self.retry_timeout_seconds)


    def check_wait_and_create(self, number_of_retries: int = 5):
        self.check_and_wait(number_of_retries=number_of_retries)
        self.create()
