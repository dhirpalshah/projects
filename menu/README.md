menu project:

initially just a basic coding question, I decided to flesh this out a bit more.
given a bunch of rules, output the order that a restaurant would receive:

breakfast
Main (1): Eggs
Side (2): Toast
Drink (3): Coffee

lunch
Main (1): Sandwich
Side (2): Chips
Drink (3): Soda

dinner
Main (1): Steak
Side (2): Potatoes
Drink (3): Wine
Dessert (4): Cake

Rules:
1. An order consists of a meal and collection of comma separated item Ids.
2. The system should return the name of the items ordered
3. The system should always return items in the following order: meal, side, drink
4. If multiple items are ordered, the number of items should be indicated
5. Each order must contain a main and a side
6. If no drink is ordered, water should be returned
7. At breakfast, multiple cups of coffee can be ordered
8. At lunch, multiple sides can be ordered
9. At dinner, dessert must be ordered
10. At dinner, water is always provided

How to run:
When directly calling

make

you will see a randomized order along with the output. Note: it may take
several tries to get a valid order.

If you want to run the given test samples, uncomment the test samples from
lines 190-205. You will see the correct output then.


interesting stuff:

The only dependency in this code is the random module. The random module's
purpose is for the random testing function, which randomly selects a mealtime
and a random amount of numbers ranging from 1 to 3 in order to create a unique
order each time. This way, you can test the function effectively.

kind of a cool way to test a bunch of edge cases on a function without having
to think too hard.

there's also a super boring makefile just because.