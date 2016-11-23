import ff
import gates
import spiceutils
import models
import logic_path
import sys
import yaml
import global_wire


class Array:
    def __init__(self, **kwargs):
        self.Na = kwargs["Na"] #1

    def generate(self):
        self.ff_stats = ff.simulate_ff()

    def get_energy(self):
        # estimate the array energy from the array simulation
        # note that the array simulation assumes activation of all the cells in
        # a wordline, so we have to divide by the global->local decoding factor
        total_energy = 0
        leakage_energy = float(self.ff_stats["e_leak"])
        ff_energy = float(self.ff_stats["e_peracc"])
        total_energy += ff_energy

        # multiple tables energy (hack to get energy for multiple tables)
        total_energy *= self.Na
        leakage_energy *= self.Na

        # hack for CAM energy
        print "Leakage Energy: ", leakage_energy

        return total_energy

if __name__ == '__main__':
    yaml_config_file = open(sys.argv[1])
    yaml_doc = yaml.load(yaml_config_file)

    # the minimum pitch version
    # min feature size
    if "technology" in yaml_doc:
        l = yaml_doc["technology"]["lambda"]*1e-6
    else:
        l = 0.022e-6

    tech_params = {}

    # arrays
    num_arrays = yaml_doc["arrays"]["num_array"]
    tech_params["Na"] = num_arrays

    array = Array(**tech_params)
    array.generate()

    #delay = array.get_delay()
    energy = array.get_energy()
    print 'Energy = ', energy
    freq = 2e9
    power = energy * freq
    print 'Power @ 2GHz = ', power
