CSCI3260 Assignment 2 Texture Mapping/ Lighting Control  

Name: YU Heyang XUE Shaohua
Student ID:1155141480 1155141685

Manipulation:
        Key "Esc": exit
        Key "Up": Move forward
        Key "Down": Move backward
        Key "Left": Move left
        Key "Right": Move right

        Mouse can chage the direction of view, which is also the front direction of the spacecraft.

        When the spacecraft is too close to the craft, the texture of craft will change into red, which means alerting.
        (The provided red and gold texture looks a little bit strange, so we add a redTexture.jpg and a golden.jpg in our texture folder)
        
        We added another kind of treasure diamond, it's texture is in the texture folder.

        When the spacecraft is close enough to the treasures(rocks), the texture of treasures(rocks) will be unbinded, which make it black.
        Also, the point of these treasures will be collected, gold = 1 point, diamond = 2point.

        When point reaches 50, the texture of spacecraft will change to gold texture, after finish all collection task, it will change to diamond texture.

        We also added dirlight and a point light which is red, the pointlight move together with the spacecraft.

        Some code are form tutorials and learnopengl.

