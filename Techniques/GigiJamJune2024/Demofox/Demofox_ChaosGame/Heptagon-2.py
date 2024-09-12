import Host
import GigiArray

Host.SetVariable("FrameIndex", "0")

# https://en.wikipedia.org/wiki/Chaos_game#Optimal_value_of_r_for_every_regular_polygon
Host.SetVariable("Fraction", "1.4")

# Calculated from https://www.mathopenref.com/coordpolycalc.html
Host.SetVariable("NumVerts", "7")
Host.SetVariable("Point0", "0.500, 0.250")
Host.SetVariable("Point1", "0.305, 0.344")
Host.SetVariable("Point2", "0.256, 0.556")
Host.SetVariable("Point3", "0.392, 0.725")
Host.SetVariable("Point4", "0.608, 0.725")
Host.SetVariable("Point5", "0.744, 0.556")
Host.SetVariable("Point6", "0.695, 0.344")
