import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DraggableDirective } from './draggable.directive';

@NgModule({
  declarations: [DraggableDirective],
  imports: [CommonModule],
  exports: [DraggableDirective],
})
export class DirectivesModule {}
