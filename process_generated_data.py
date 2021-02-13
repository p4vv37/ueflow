import time
# time.sleep(60*60*3)

import os
# --- Uncomment to use only CPU (e.g. GPU memory is too small)
# os.environ["CUDA_DEVICE_ORDER"] = "PCI_BUS_ID"   # see issue #152
# os.environ["CUDA_VISIBLE_DEVICES"] = "-1"

# import sys
# sys.path.append("/usr/local/cuda-10.1/bin")
# os.environ["LD_LIBRARY_PATH"] = "/usr/local/cuda-10.1/lib64"

import tensorflow as tf

tf.test.is_gpu_available(cuda_only=True)

# reate logger - nocer formatting
import logging

logger = logging.getLogger()
logging._warn_preinit_stderr = 0
logger.setLevel(logging.INFO)
formatter = logging.Formatter('%(asctime)s: %(message)s')
ch = logging.StreamHandler()
ch.setFormatter(formatter)
logger.handlers = [ch]
logger.info("Logger started.")

import sqlite3
import numpy as np


def adapt_array(arr):
    """
    Save Numpy array to SqLite.
    Source:
    http://stackoverflow.com/a/31312102/190597 (SoulNibbler)
    """
    out = io.BytesIO()
    np.save(out, arr)
    out.seek(0)
    return sqlite3.Binary(out.read())


def convert_array(text):
    """
    Load Numpy array from Sqlite.
    Source:
    http://stackoverflow.com/a/31312102/190597 (SoulNibbler)
    """
    out = io.BytesIO(text)
    out.seek(0)
    return np.load(out)


sqlite3.register_adapter(np.ndarray, adapt_array)
sqlite3.register_converter("array", convert_array)
data_path = os.path.join(os.getcwd(), "samplesGeneration", "prepared_data.bd")
data_path = "E:/tmp/data_2021_5.bd"
source = sqlite3.connect(data_path, detect_types=sqlite3.PARSE_DECLTYPES, check_same_thread=False)
db = sqlite3.connect(':memory:', detect_types=sqlite3.PARSE_DECLTYPES)
source.backup(db)

import io
sqlite3.register_adapter(np.ndarray, adapt_array)
sqlite3.register_converter("array", convert_array)

db_cursor = db.cursor()
sql_query = "SELECT COALESCE(MAX(id)+1, 0) FROM data"
db_cursor.execute(sql_query)
number_of_samples = db_cursor.fetchone()[0]
print("number of samples: ", number_of_samples)

sql_query = "SELECT x_delta FROM data WHERE id == 1"
db_cursor.execute(sql_query)

import tensorflow.keras as keras


class DataGenerator(keras.utils.Sequence):
    'Generates data for Keras from SQLite database'

    def __init__(self, db, indexes=None, batch_size=25,
                 shuffle=True, dim=(3, 39, 1), dim_y=(36, 1), y_name="y_delta"):
        'Initialization'
        if indexes is None:
            raise Exception("Indexes need to be provided!")
        sqlite3.register_adapter(np.ndarray, self.adapt_array)
        sqlite3.register_converter("array", self.convert_array)
        self.y_name = y_name
        self.dim = dim
        self.dim_y = dim_y
        self.db = db
        self.db_cursor = self.db.cursor()
        self.N = len(indexes) - 1
        self.sample_index = indexes
        self.batch_size = int(batch_size)
        self.shuffle = shuffle
        self.on_epoch_end()

    def __del__(self):
        pass
        # self.db.close()

    def __len__(self):
        'Denotes the number of batches per epoch'
        return int(np.floor(self.N / self.batch_size))

    def __getitem__(self, index):
        'Generate one batch of data - generates indexes of the batch'
        # Generate indexes of the batch
        samples_batch = np.arange((index) * self.batch_size, (index + 1) * self.batch_size)

        # Generate data
        while True:
            try:
                x_delta, x_abs_0, x_abs, x_force, y = self.__data_generation(samples_batch)
                break
            except ValueError:
                continue

        return ({"input_delta": x_delta, "input_abs": x_abs, "input_abs_0": x_abs_0, "input_force" : x_force}, y)

    def on_epoch_end(self):
        'Updates indexes after each epoch'
        if self.shuffle:
            np.random.shuffle(self.sample_index)

    def __data_generation(self, samples_batch):
        'Generates data containing batch_size samples'  # X : (n_samples, *dim, n_channels)
        # Initialization
        x_delta = np.empty((self.batch_size, 4, 6))
        x_abs = np.empty((self.batch_size, 4, 9))
        x_abs_0 = np.empty((self.batch_size, 4, 9))
        x_force = np.empty((self.batch_size, 3))
        y = np.empty((self.batch_size, *self.dim_y))
        inds = self.sample_index[samples_batch]
        db_cursor = self.db_cursor
        sql_query = "SELECT x_delta, x_abs_0, x_abs, x_force, {y_name} FROM data WHERE id in ({index})". \
            format(y_name=self.y_name, index=','.join(str(ind) for ind in inds))
        db_cursor.execute(sql_query)
        try:
            for i, line in zip(range(self.batch_size), db_cursor.fetchall()):
                if line is None:
                    x_delta[i, :] = x_delta[i - 1, :]
                    x_abs_0[i, :] = x_abs_0[i - 1, :]
                    x_abs[i, :] = x_abs[i - 1, :]
                    x_force[i, :] = x_force[i - 1, :]
                    y[i] = y[i - 1]
                    continue  # Bad, temporary solution
                x_delta[i, :] = np.array(line[0])
                x_abs_0[i, :] = np.array(line[1])
                x_abs[i, :] = np.array(line[2])
                x_force[i, :] = np.array(line[3])
                y[i, :] = np.array(line[4])
        except TypeError as e:
            print(sql_query)
            raise e
        return x_delta, x_abs_0, x_abs, x_force, y

    def adapt_array(self, arr):
        """
        Save Numpy array to SqLite.
        Source:
        http://stackoverflow.com/a/31312102/190597 (SoulNibbler)
        """
        out = io.BytesIO()
        np.save(out, arr)
        out.seek(0)
        return sqlite3.Binary(out.read())

    def convert_array(self, text):
        """
        Load Numpy array from Sqlite.
        Source:
        http://stackoverflow.com/a/31312102/190597 (SoulNibbler)
        """
        out = io.BytesIO(text)
        out.seek(0)
        return np.load(out)


test_train_ratio = 0.1

test_size = int(test_train_ratio * number_of_samples)

indexes = np.arange(0, number_of_samples - 1)
np.random.shuffle(indexes)
train_indexes, val_indexes = indexes[:-test_size], indexes[test_size:]

training_generator = DataGenerator(db, indexes=train_indexes, batch_size=8, dim=(3, 39), dim_y=(4, 6))
validation_generator = DataGenerator(db, indexes=val_indexes, batch_size=8, dim=(3, 39), dim_y=(4, 6))

training_generator[0]
training_generator[1]
training_generator[2]
