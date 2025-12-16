// void updateButton(
//     UIButton& btn,
//     float mouseX, float mouseY,
//     bool mouseDown
// )
// {
//     btn.clicked = false;

//     bool inside =
//         mouseX >= btn.pos.x &&
//         mouseX <= btn.pos.x + btn.size.x &&
//         mouseY >= btn.pos.y &&
//         mouseY <= btn.pos.y + btn.size.y;

//     if (inside)
//     {
//         if (mouseDown)
//         {
//             btn.state = UIButtonState::Pressed;
//         }
//         else
//         {
//             if (btn.state == UIButtonState::Pressed)
//                 btn.clicked = true;

//             btn.state = UIButtonState::Hover;
//         }
//     }
//     else
//     {
//         btn.state = UIButtonState::Normal;
//     }
// }
