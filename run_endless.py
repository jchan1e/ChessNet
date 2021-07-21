import os
import sys
import math
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

def mutateLayers(Layers, mutate, rate):
    # <rate> chance per layer to add/remove neurons
    L = Layers[:]
    #for num in L:
    for i in range(len(L)):
        num = L[i]
        if random.random() < mutate:
            if random.randrange(2) == 0:
                L[i] = num+1 if num <= 5 else int(num * rate)
            else:
                L[i] = max(1, int(num / rate))
    # <ratei^2> chance to add/remove a layer at a random place in the stack
    if random.random() < mutate*mutate:
        if random.randrange(2) == 0:
            if len(L) > 1:
                del L[random.randrange(len(L))]
        else:
            slot = random.randrange(len(L) + 1)
            if slot == 0:
                L.insert(slot, L[0])
            elif slot == len(L):
                L.append(L[-1])
            else:
                avg = int((L[slot-1] + L[slot]) / 2)
                L.insert(slot, avg)
    return L

generation = 0
turn_time = 2

if len(sys.argv) > 1:
    turn_time = int(sys.argv[1])
if len(sys.argv) > 2:
    generation = int(sys.argv[2])

def simgame(agent1, agent2, generation):
    print("Simulating {} vs {}".format(agent1, agent2))
    #if int(agent1) < int(agent2):
    if not os.path.isfile("gamelogs/gen{2}/{0}.{1}.game".format(agent1, agent2, generation)):
        if not os.path.isfile("gamelogs/gen{2}/{1}.{0}.game".format(agent1, agent2, generation)):
            if random.randrange(2) == 0:
                os.system("./simgame Agents/{0}.agent Agents/{1}.agent gamelogs/gen{3}/{0}.{1}.game {2}".format(agent1, agent2, turn_time, generation))
            else:
                os.system("./simgame Agents/{0}.agent Agents/{1}.agent gamelogs/gen{3}/{0}.{1}.game {2}".format(agent2, agent1, turn_time, generation))

    winner = 0
    if os.path.isfile("gamelogs/gen{2}/{0}.{1}.game".format(agent1, agent2, generation)):
        winner = float(os.popen("head -1 gamelogs/gen{2}/{0}.{1}.game".format(agent1, agent2, generation)).read().strip())
    else:
        winner = float(os.popen("head -1 gamelogs/gen{2}/{0}.{1}.game".format(agent2, agent1, generation)).read().strip())
    return (agent1, agent2, winner)

def train(target):
    print("Training {}".format(target))
    os.system("./train Agents/" + target + ".agent gamelogs/gen*/*.game > Agents/" + target + ".log")

#pool_size = 1
#pool_size = os.cpu_count()/2
pool_size = int(max((os.cpu_count()-2)/2, 2))

agent_nums = [int(os.path.split(a[:-6])[1]) for a in filter(lambda s: ".agent" in s, os.listdir("Agents/") + os.listdir("Agents/archive/"))]
next_agent_num = max(agent_nums) + 1

pool = multiprocessing.Pool(pool_size)
pool2 = multiprocessing.Pool(pool_size*2)

while generation >= 0:
    print("Generation: ", generation)
    # simulate games between all member of current population
    #   save outputs to gamelogs folder
    print("Gathering agents")
    agents = [a[:-6] for a in filter(lambda s: ".agent" in s, os.listdir("Agents/"))]
    wins = {agent:0.0 for agent in agents}
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

    print("Simulating games")
    args = list(itertools.combinations(agents, 2))
    args = [(tup[0], tup[1], generation) for tup in args]
    random.shuffle(args)
    args = args[:int(len(args)/2)]

    results = pool.starmap(simgame, args, chunksize=1)
    #print(results)
    #exit(0)
    #results = []
    #for arg1, arg2 in args:
    #    results.append(simgame(arg1, arg2))

    for (agent1, agent2, winner) in results:
        wins[agent1] += winner - 0.5
        wins[agent2] += 0.5 - winner

    # rotate out old logs
    if generation >= 3:
        print("Rotating logs from gen{}".format(generation-3))
        os.system("mv gamelogs/gen{} gamelogs/archive/".format(generation-3))

    # cull lowest performing 50% of agents
    print("Culling agents:")
    num_agents = len(agents)
    removed = []
    for i in range(int(num_agents/2)):
        lowest = float('inf')
        lagent = ""
        for agent in agents:
            if wins[agent] < lowest and agent not in removed:
                lagent = agent
                lowest = wins[agent]
        os.system("mv Agents/{}.* Agents/archive/".format(lagent))
        removed.append(lagent)
    print(removed)
    survivors = [a for a in agents if a not in removed]

    # generate new agents
    print("Generating new agents")
    targets = []
    for i in range(int(num_agents/2)):
        #* Method 1 - 2 parent genetics
        #  Method 2 - whole population as parents
        #  Method 3 - parthenogenesis
        #  Method 4 - totally original

        # pick two random agents
        selection = random.sample(survivors, 2)
        parentstrings = []
        parents = []
        with open("Agents/{}.agent".format(selection[0]), "r") as f:
            parentstrings.append(f.readlines())
        with open("Agents/{}.agent".format(selection[1]), "r") as f:
            parentstrings.append(f.readlines())
        for p in parentstrings:
            p = [s.strip() for s in p]
            p[5] = [int(s) for s in p[5].split(" ")]
            parents.append(p)

        # fill in values from each
        alpha  = float(random.choice(parents)[0])
        decay  = float(random.choice(parents)[1])
        #bias   = float(random.choice(parents)[2])
        bias   = math.sqrt(2.0)
        mutate = float(random.choice(parents)[3])
        layers = combineLayers(parents[0][5], parents[1][5])

        # mutate
        rate = 2 ** (0.125)
        layers = mutateLayers(layers, mutate, rate**2)
        if random.random() < mutate:
            if random.randrange(2) == 0:
                alpha /= rate
            else:
                alpha *= rate
        if random.random() < mutate:
            if random.randrange(2) == 0:
                decay /= rate
            else:
                decay *= rate
        #if random.random() < mutate:
        #    if random.randrange(2) == 0:
        #        bias = (bias-1)/rate + 1
        #    else:
        #        bias = (bias-1)*rate + 1
        if random.random() < mutate:
            if random.randrange(2) == 0:
                mutate /= rate
            else:
                mutate *= rate

        args = " ".join([str(arg) for arg in [alpha, decay, bias, mutate]]) + " " + " ".join([str(layer) for layer in layers])
        cmd = "./createAgent.sh Agents/" + str(next_agent_num).zfill(4) + " " + args
        os.system(cmd)

        # train
        targets.append(str(next_agent_num).zfill(4))
        #os.system("./train Agents/" + next_agent_num + ".agent")

        next_agent_num += 1

    #bulk train (parallelize me)
    #for target in targets:
    #    os.system("./train Agents/" + target + ".agent > Agents/" + target + ".log")
    pool2.map(train, targets, chunksize=1)

    generation += 1

