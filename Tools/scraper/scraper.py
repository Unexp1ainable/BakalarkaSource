NAME = "Tools/scraper/log.txt"

with open(NAME) as file:
    worked_on = []
    for line in file:
        line = line.strip()
        if "Started with value  " in line:
            worked_on.append(line[20:])
        elif "convergence reached " in line:
            worked_on.remove(line[20:])
        elif "Error, no pixels of interest. " in line:
            worked_on.remove(line[30:])

print(worked_on)
