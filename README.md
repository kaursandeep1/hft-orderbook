# HFT Order Book System

Just a simple order book I built while learning about trading systems. It matches buy and sell orders – the same thing that happens behind the scenes when you trade stocks.

# What's inside?
The code keeps two lists:

- Bids – people wanting to buy (highest price first)

- Asks – people wanting to sell (lowest price first)

When orders come in, they get added to the right list. The system always knows the best price on each side instantly.

# Does it actually work?
Run it and see:

``git clone https://github.com/kaursadneep1/orderbook-hft
cd orderbook-hft
mkdir build && cd build
cmake ..
make
./orderbook_demo``

You'll watch orders get added, executed, cancelled – then a benchmark shows how fast it runs.

# How fast?
On my Mac:

``1 million orders → 33 milliseconds → ~30 million per second``

Fast enough for most things. Could probably make it faster with some tweaks.

# Want to improve it?

Things I'm thinking about adding:

Real matching engine (auto-execute when prices cross)

Connect to actual market data

Try a lock-free version

Add a simple web view to see the book move
