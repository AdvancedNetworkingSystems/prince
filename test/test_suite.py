import ConfigParser
import numpy as np
from prince_test_oonf import PrinceTestOONF
import subprocess


class TestSuite:
    def set_test_params(self, heuristic, implementation, weight, port, sample_size, sample_interval, iterations, g_type):
        self.heuristic = heuristic
        self.implementation = implementation
        self.weight = weight
        self.port = port
        self.sample_size = sample_size
        self.sample_interval = sample_interval
        self.iterations = iterations
        self.g_type = g_type
        self.test_id = "data/" + implementation + "_" + str(heuristic) + "_" + str(iterations * sample_interval) + "_" + str(g_type)

    def write_config(self):
        json_config = '''
{
    "proto": {
        "protocol": "oonf",
        "host": "127.0.0.1",
        "port": %i,
        "timer_port": %i,
        "refresh": 0,
        "log_file": "data/prova.log"
    },
    "graph-parser": {
        "heuristic": %i,
        "weights": %i,
        "recursive": 0,
        "stop_unchanged": 0,
        "multithreaded": 0
    }
}
        '''
        with open(self.cfg_filename, 'w') as cfg_file:
            print >> cfg_file, json_config % (self.port, self.port, self.heuristic, self.weight)


    def battery(self):
        '''
        Run a battery of tests
        heuristic: 1 to run prince using the heuristic for the bc, 0 to run without heuristic(slow)
        implementation: filename with path of the prince executable
        port: prot to use for the communication between prince and this test
        sample_size:
        sample_interval:
        iter:
        g_type: 0 for CN, 1 for PLAW, 2 for NPART 3 for GRID
        '''
        p = PrinceTestOONF()
        x = []
        result_file = open(self.test_id + ".dat", 'a')
        weight = 0
        # run prince w heuristic
        self.cfg_filename =self.test_id + ".json"
        self.write_config()
        proc = subprocess.Popen("exec" + " " + self.implementation + " " + self.cfg_filename, shell=True)
        # cycle till the max values
        battery_measures = np.zeros((1, 6)).squeeze()
        for i in range(1, self.iterations + 1):
            size = self.sample_interval * i
            # run tests with heuristic
            measures_mtx = p.test(self.g_type, size, self.sample_size, self.port)
            measures_mean = measures_mtx.mean(axis=0)
            measures_var = measures_mtx.std(axis=0)
            measures = np.concatenate((measures_mean, measures_var), axis=0)
            line = "%d %f %f %f %f %f %f\n" % (size, measures[0], measures[1], measures[2], measures[3], measures[4], measures[5])
            result_file.write(line)
            result_file.flush()
            print line
            battery_measures = np.c_[battery_measures, measures]
        print("Exec Time mean  Hello relative error mean  TC relative error mean, Exec Stdev  hello std  tc std")
        print(np.delete(battery_measures, 0, axis=1))
        result_file.close()
        p.terminate_workers()
        proc.terminate()
