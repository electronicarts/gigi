import Host
import GigiArray

Host.SetVariable("FrameIndex", "0")

# https://en.wikipedia.org/wiki/Chaos_game#Optimal_value_of_r_for_every_regular_polygon
Host.SetVariable("Fraction", "0.692")

# Calculated from https://www.mathopenref.com/coordpolycalc.html
Host.SetVariable("NumVerts", "7")
Host.SetVariable("Point0", "0.500, 0.000")
Host.SetVariable("Point1", "0.109, 0.188")
Host.SetVariable("Point2", "0.013, 0.611")
Host.SetVariable("Point3", "0.283, 0.950")
Host.SetVariable("Point4", "0.717, 0.950")
Host.SetVariable("Point5", "0.987, 0.611")
Host.SetVariable("Point6", "0.891, 0.188")
