import os
import sys
import random
import multiprocessing
import itertools

def combineLayers(L1, L2): # L1 and L2 are lists of ints
    L = []
    for i in range(max(len(L1), len(L2))):
        source = random.choice([L1,L2])
        if i < len(source):
            L.append(source[i])
    return L

def mutateLayers(L, mutate, rate):
    # <rate> chance per layer to add/remove neurons
    for num in L:
        if random.random() < mutate:
            if randrange(2) == 0:
                num = num <= 5 ? num+1 : int(num * rate)
            else:
                num = max(1, int(num / rate))
    # <rate> chance to add/remove a layer at a random place in the stack
    if random.random() < mutate:
        if randrange(2) == 0:
            if len(L) > 1:
                del L[randrange[len(L)]]
        else:
            slot = randrange(len(L) + 1)
            if slot == 0:
                L.insert(slot, L[0])
            elif: slot == len(L):
                L.append(L[-1])
            else:
                avg = (L[slot] + L[slot+1]) / 2
                L.insert(slot, avg)
    return L

popName = "Population"
generation = 0
turn_time = 5

#pool_size = 8
#pool_size = os.cpu_count()/2
pool_size = max(os.cpu_count()-4, 2)

agent_nums = [int(os.path.split(a[:-6])[1]) for a in filter(lambda s: ".agent" in s, os.listdir("Agents/") + os.listdir("Agents/archive/"))]
next_agent_num = max(agent_nums) + 1

if len(sys.argv) > 1:
    turn_time = int(sys.argv[1])
if len(sys.argv) > 2:
    generation = int(sys.argv[2])

pool = multiprocessing.Pool(pool_size)

while generation < 1:
    # simulate games between all member of current population
    #   save outputs to gamelogs folder
    agents = [a[:-6] for a in filter(lambda s: ".agent" in s, os.listdir("Agents/"))]
    wins = {agent:0 for agent in agents}
    os.system("mkdir gamelogs/gen{}".format(generation))
    # (Parallelize me)
    #for agent1 in agents:
    #    for agent2 in agents:
    #        #if int(agent1) < int(agent2):
    #        os.system("./simgame Agents/{0}.agent Agents/{1}.agent {2} > gamelogs/gen{3}/{0}.{1}.game".format(agent1, agent2, turn_time, generation))
    #        winner = int(os.popen("head -1 gamelogs/gen{2}/{0}.{1}.game".format(agent1, agent2, generation)).read())
    #        l.acquire()
    #        wins[agent1] += winner
    #        wins[agent2] += 1-winner
    #        l.release()

    args = list(itertools.product(agents, agents))
    def simgame(agent1, agent2):
        #if int(agent1) < int(agent2):
        os.system("./simgame Agents/{0}.agent Agents/{1}.agent {2} > gamelogs/gen{3}/{0}.{1}.game".format(agent1, agent2, turn_time, generation))
        winner = int(os.popen("head -1 gamelogs/gen{2}/{0}.{1}.game".format(agent1, agent2, generation)).read())
        return (agent1, agent2, winner)

    results = pool.starmap(simgame, args)

    for (agent1, agent2, winner) in results:
        wins[agent1] += winner
        wins[agent2] += 1-winner

    # rotate out old logs
    if generation >= 3:
        os.system("mv gamelogs/gen{} gamelogs/archive/".format(generation-3))

    # cull lowest performing 50% of agents
    num_agents = len(agents)
    survivors = agents.copy()
    for i in range(num_agents/2):
        lowest = float('inf')
        lagent = ""
        for agent in agents:
            if wins[agent] < lowest:
                lagent = agent
                lowest = wins[agent]
        os.system("mv Agents/{}.* archive/".format(agent))
        del survivors[lagent]

    # generate new agents
    targets = []
    for i in range(num_agents/2):
        #* Method 1 - 2 parent genetics
        #  Method 2 - whole population as parents
        #  Method 3 - parthenogenesis
        #  Method 4 - totally original

        # pick two random agents
        selection = random.sample(survivors, 2)
        parents = []
        with open("Agents/{}.agent".format(selection[0]), r) as f:
            parents.append(f.readlines())
        with open("Agents/{}.agent".format(selection[1]), r) as f:
            parents.append(f.readlines())
        for p in parents:
            p = [s.strip() for s in p]
            p[5] = [int(s) for s in p[5].split()]

        # fill in values from each
        alpha  = float(random.choice(parents)[0])
        decay  = float(random.choice(parents)[1])
        bias   = float(random.choice(parents)[2])
        mutate = float(random.choice(parents)[3])
        layers = combineLayers(parents[0][5], parents[1][5])

        # mutate
        rate = 2 ** (0.0625)
        layers = mutateLayers(layers, mutate, rate**4)
        if random.random() < mutate:
            if randrange(2) == 0:
                alpha /= rate
            else:
                alpha *= rate
        if random.random() < mutate:
            if randrange(2) == 0:
                decay /= rate
            else:
                decay *= rate
        if random.random() < mutate:
            if randrange(2) == 0:
                bias = (bias-1)/rate + 1
            else:
                bias = (bias-1)*rate + 1
        if random.random() < mutate:
            if randrange(2) == 0:
                mutate /= rate
            else:
                mutate *= rate

        args = " ".join([str(arg) for arg in [alpha, decay, bias, mutate]]) + " ".join([str(layer) for layer in layers])
        cmd = "./createNN Agents/" + str(next_agent_num).zfill(4) + " " + args
        os.system(cmd)

        # train
        targets.append(str(next_agent_num).zfill(4))
        #os.system("./train Agents/" + next_agent_num + ".agent")

        next_agent_num += 1

    #bulk train (parallelize me)
    #for target in targets:
    #    os.system("./train Agents/" + target + ".agent > Agents/" + target + ".log")
    def train(target):
        os.system("./train Agents/" + target + ".agent > Agents/" + target + ".log")
    pool.map(train, targets)

